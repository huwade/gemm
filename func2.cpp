#include <string.h>
#include <map>
#include <vector>
#include <cstddef>
#include <iostream>
#include <memory>
#include <bits/stdc++.h>
#include <stdlib.h>     /* malloc, free, rand */



enum Padding {
  VALID = 1,  // No padding.
  SAME = 2,   // Input and output layers have the same size.
};

const size_t kMaxChunkSize = (16 * 1024 * 1024);





template <class T, size_t size>
struct Im2ColBufferResource 
{
  Im2ColBufferResource<T, size>() 
  {
    data = static_cast<T*>(malloc(size * sizeof(T)));
  }
  ~Im2ColBufferResource<T, size>() { free(data); }
  // This mutex ensures that only a single operation at a time is able to use
  // the buffer memory held by this resource.
    
  //mutex mu;
  T* data;
  //string DebugString() { return "Im2ColBufferResource"; }
};

// With that in mind, I've avoided using anything except pretty standard C++
// types. This is especially noticeable in the data access through raw array
// indexing. It's deliberate in this case though, since it makes the underlying
// memory order very explicit, which is important for both inspecting memory
// contents during debugging and for specifying what we expect to others.
// The memory layout of the data is, from biggest stride to smallest:
// input_data = [input_batches, input_height, input_width, input_depth]
// filter_data = [filter_height, filter_width, input_depth, filter_count]
// output_data = [input_batches, output_height, output_width, filter_count]

class ReferenceConvFunctor 
{
 public: 
  void operator()(const int* input_data,
                  int input_batches, int input_height, int input_width,
                  int input_depth, const int* filter_data, int filter_height,
                  int filter_width, int filter_count, int stride_rows,
                  int stride_cols, Padding padding, int* output_data,
                  int output_height, int output_width) {
    // The two different padding modes we support can be a bit confusing. SAME
    // means we're trying to produce an output image that's the same size as the
    // input. It's complicated by stride, which shrinks the output image by a
    // a factor, but it means we end up sampling from outside the borders of the
    // input. These out-of-bounds values are read as zeroes. VALID means only
    // produce output values where the filters can read all their values from
    // within the input image. It effectively removes the margins of the output
    // image compared to the one produced by SAME. Stride complicates this
    // definition though, because it can result in the right and bottom filter
    // patches sampling from outside the borders if it's greater than 1.
    // Most of the logic for sorting this all out is done before this function,
    // when we calculate the output size, but the positioning of the origin of
    // the filters is different between the two modes, since SAME positions the
    // first filter off the edge of the input.
    int filter_left_offset;
    int filter_top_offset;
    if (padding == VALID) {
      filter_left_offset =
          ((output_width - 1) * stride_cols + filter_width - input_width + 1) /
          2;
      filter_top_offset = ((output_height - 1) * stride_rows + filter_height -
                           input_height + 1) /
                          2;
    } else {
      filter_left_offset =
          ((output_width - 1) * stride_cols + filter_width - input_width) / 2;
      filter_top_offset =
          ((output_height - 1) * stride_rows + filter_height - input_height) /
          2;
    }

    // If we've got multiple images in our input, work through each of them.
    for (int batch = 0; batch < input_batches; ++batch) {
      // Walk through all the output image values, sliding the filter to
      // different positions in the input.
      for (int out_y = 0; out_y < output_height; ++out_y) {
        for (int out_x = 0; out_x < output_width; ++out_x) {
          // Each filter kernel produces one output channel.
          for (int out_channel = 0; out_channel < filter_count; ++out_channel) {
            // We're going to calculate a single output value, which means we
            // need to multiply a three dimensional kernel of weights against
            // the current location within the input image.
            /*
             *-------------------------------...
             |\ ^
             | \in_depth
             |  \ v
             |   *-------------------------------...
             |   |            ^
             |   |       in_y_origin
             |   |            v   \
             |   |<in_x_origin>*---*^
             |   |            \|   |filter_height
             .   |             *---*v
             .   |             <--->
             .         filter_width
             .
            */
            const int in_x_origin = (out_x * stride_cols) - filter_left_offset;
            const int in_y_origin = (out_y * stride_rows) - filter_top_offset;
            int total(0);
            for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
              for (int filter_x = 0; filter_x < filter_width; ++filter_x) {
                for (int in_channel = 0; in_channel < input_depth;
                     ++in_channel) {
                  const int in_x = in_x_origin + filter_x;
                  const int in_y = in_y_origin + filter_y;
                  int input_value;
                  // If the location is outside the bounds of the input image,
                  // use zero as a default value.
                  if ((in_x >= 0) && (in_x < input_width) && (in_y >= 0) &&
                      (in_y < input_height)) {
                    input_value =
                        input_data[(batch * input_height * input_width *
                                    input_depth) +
                                   (in_y * input_width * input_depth) +
                                   (in_x * input_depth) + in_channel];
                  } else {
                    input_value = int(0);
                  }
                  const int filter_value =
                      filter_data[(filter_y * filter_width * input_depth *
                                   filter_count) +
                                  (filter_x * input_depth * filter_count) +
                                  (in_channel * filter_count) + out_channel];
                  total += (input_value * filter_value);
                }
              }
            }
            output_data[(batch * output_height * output_width * filter_count) +
                        (out_y * output_width * filter_count) +
                        (out_x * filter_count) + out_channel] = total;
          }
        }
      }
    }
  }
};


// Implements convolution as a two stage process, first packing the patches of
// the input image into columns (im2col) and then running GEMM to produce the
// final result.


void Im2ColConvFunctor (const int* input_data, int input_batches, 
                  int input_height, int input_width,
                  int input_depth, const int* filter_data, int filter_height,
                  int filter_width, int filter_count, int stride_rows,
                  int stride_cols, Padding padding, int* output_data,
                  int output_height, int output_width) 
{
   

    

    // These calculations define how the patches will be positioned within the
    // input image. The actual definitions are quite complex, and rely on the
    // previously-calculated output size.
    int filter_left_offset;
    int filter_top_offset;
    if (padding == VALID) {
      filter_left_offset =
          ((output_width - 1) * stride_cols + filter_width - input_width + 1) /
          2;
      filter_top_offset = ((output_height - 1) * stride_rows + filter_height -
                           input_height + 1) /
                          2;
    } else {
      filter_left_offset =
          ((output_width - 1) * stride_cols + filter_width - input_width) / 2;
      filter_top_offset =
          ((output_height - 1) * stride_rows + filter_height - input_height) /
          2;
    }

    // The im2col buffer has # of patches rows, and # of filters cols.
    // It's laid out like this, in row major order in memory:
    //        < filter value count >
    //   ^   +---------------------+
    // patch |                     |
    // count |                     |
    //   v   +---------------------+
    // Each patch row contains a filter_width x filter_height patch of the
    // input, with the depth channel as the most contiguous in memory, followed
    // by the width, then the height. This is the standard memory order in the
    // image world if it helps to visualize it.
    const int filter_value_count = filter_width * filter_height * input_depth;

    const int patches_per_chunk =
        kMaxChunkSize / (filter_value_count * sizeof(int));

    const int chunk_value_count =
        (kMaxChunkSize + (sizeof(int) - 1)) / sizeof(int);
    // Because memory allocation is very expensive on mobile platforms, try to
    // allocate a persistent buffer that will be kept around between calls. We
    // use TensorFlow's resource management to ensure that the memory will be
    // released when the session is over.
    Im2ColBufferResource<int, chunk_value_count>* im2col_buffer_resource;
    

    int* im2col_buffer = im2col_buffer_resource->data;

    const int patch_count = (input_batches * output_height * output_width);
    const int chunk_count =
        (patch_count + (patches_per_chunk - 1)) / patches_per_chunk;
    for (int chunk_index = 0; chunk_index < chunk_count; ++chunk_index) {
      const int patch_index_start = chunk_index * patches_per_chunk;
      const int patch_index_end =
          std::min(patch_index_start + patches_per_chunk, patch_count);
      for (int patch_index = patch_index_start; patch_index < patch_index_end;
           ++patch_index) {
        const int batch = patch_index / (output_height * output_width);
        const int out_y = (patch_index / output_width) % output_height;
        const int out_x = patch_index % output_width;
        const int* input_batch_start =
            input_data + (batch * input_height * input_width * input_depth);
        const int in_y_origin = (out_y * stride_rows) - filter_top_offset;
        const int in_x_origin = (out_x * stride_cols) - filter_left_offset;
        const int patch_index_within_chunk = patch_index % patches_per_chunk;
        int* im2col_patch_start =
            im2col_buffer + (patch_index_within_chunk * filter_value_count);
        for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
          const int in_y = in_y_origin + filter_y;
          int* im2col_row_start =
              im2col_patch_start + (filter_y * filter_width * input_depth);
          // If we're off the top or the bottom of the input, fill the
          // whole row with zeroes.
          if ((in_y < 0) || (in_y >= input_height)) {
            int* im2col_row_end =
                im2col_row_start + (filter_width * input_depth);
            std::fill(im2col_row_start, im2col_row_end, int(0));
          } else {
            // What we're doing here is trying to copy and fill the im2col
            // buffer as efficiently as possible, using functions to set or
            // duplicate values en masse. We know we don't have to worry about
            // vertical edges because we dealt with that case above, so we
            // just need to handle filters that overlap the left or right
            // edges. Here's what that looks like:
            //
            // < left_zero_count > < center_copy_count > < right_zero_count >
            // +------------------+---------------------+--------------------+
            // |     (filter)     |       (image)       |      (filter)      |
            // +------------------+---------------------+--------------------+
            // in_x_origin        0                 input_width       in_x_end
            //
            // In reality it's unlikely that a filter patch will be wider
            // than an input, but this shows all the edge cases.
            // We use std::fill() to set the left and right sections to zeroes
            // and std::copy() to copy over the input data for the center.
            const int in_x_end = in_x_origin + filter_width;
            const int left_zero_count = std::max(0, 0 - in_x_origin);
            const int right_zero_count = std::max(0, in_x_end - input_width);
            const int center_copy_count =
                filter_width - (left_zero_count + right_zero_count);
            if (left_zero_count > 0) {
              int* im2col_left_start = im2col_row_start;
              int* im2col_left_end =
                  im2col_left_start + (left_zero_count * input_depth);
              std::fill(im2col_left_start, im2col_left_end, int(0));
            }
            if (center_copy_count > 0) {
              const int* input_row_start =
                  input_batch_start + (in_y * input_width * input_depth) +
                  (std::max(0, in_x_origin) * input_depth);
              const int* input_row_end =
                  input_row_start + (center_copy_count * input_depth);
              int* im2col_center_start =
                  im2col_row_start + (left_zero_count * input_depth);
              std::copy(input_row_start, input_row_end, im2col_center_start);
            }
            if (right_zero_count > 0) {
              int* im2col_right_start =
                  im2col_row_start +
                  ((left_zero_count + center_copy_count) * input_depth);
              int* im2col_right_end =
                  im2col_right_start + (right_zero_count * input_depth);
              std::fill(im2col_right_start, im2col_right_end, int(0));
            }
          }
        }
      }
      // Now we've assembled a set of image patches into a matrix, apply a
      // GEMM matrix multiply of the patches as rows, times the filter
      // weights in columns, to get partial results in the output matrix.
      const int how_many_patches = patch_index_end - patch_index_start;
      const int m = how_many_patches;
      const int n = filter_count;
      const int k = filter_value_count;
      const int lda = filter_value_count;
      const int ldb = filter_count;
      const int ldc = filter_count;
      
      int* chunk_output_data = output_data + (patch_index_start * filter_count);
      
      //TGemmFunctor gemm_functor;
      //gemm_functor(context, m, n, k, im2col_buffer, lda, filter_data, ldb,
                   //chunk_output_data, ldc);
    }
};




int main()
{
    
    const int input_depth   = 1;
    const int input_width   = 4;
    const int input_height  = 4;
    const int input_batches = 1;
    // The image matrix is:
    // |  1 |  2 |  3 |  4 |
    // |  5 |  6 |  7 |  8 |
    // |  9 | 10 | 11 | 12 |
    // | 13 | 14 | 15 | 16 |
    std::vector<int> input_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    
    // The filter matrix is:
    // | 1 | 4 | 7 |
    // | 2 | 5 | 8 |
    // | 3 | 6 | 9 |
    
    const int filter_count  = 1;
    const int filter_height = 3;
    const int filter_width  = 3;
    std::vector<int> filter_data = {1, 4, 7, 2, 5, 8, 3, 6, 9};

    const int stride_rows = 1;
    const int stride_cols = 1;
    Padding padding = VALID;
    
    const int output_height = (input_height-filter_height)/stride_rows+1; 
    const int output_width = (input_width-filter_width)/stride_cols+1; 

    // output matrix shape should be 9x4    
        
    //std::unique_ptr<int[]> output_data(new int[output_height * output_width]);
    std::vector<int> output_data;

    Im2ColConvFunctor(input_data.data(), input_batches, 
                            input_height, input_width, input_depth, 
                            filter_data.data(), filter_height, filter_width, filter_count, stride_rows,
                            stride_cols, padding, output_data.data(), output_height, output_width); 
 
    return 0;
}


