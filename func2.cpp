#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


// Function uses casting from int to unsigned to compare if value of
// parameter a is greater or equal to zero and lower than value of
// parameter b. The b parameter is of type signed and is always positive,
// therefore its value is always lower than 0x800... where casting
// negative value of a parameter converts it to value higher than 0x800...
// The casting allows to use one condition instead of two.
inline bool is_a_ge_zero_and_a_lt_b(int a, int b) {//若a大于等于零或小于b，返回true，否则返回false
  return static_cast<unsigned>(a) < static_cast<unsigned>(b);
}



template <typename Dtype>
void im2col_cpu(const Dtype* data_im, const int channels,
    const int height, const int width, const int kernel_h, const int kernel_w,
    const int pad_h, const int pad_w,
    const int stride_h, const int stride_w,
    const int dilation_h, const int dilation_w,
    Dtype* data_col) 
{
  const int output_h = (height + 2 * pad_h -
    (dilation_h * (kernel_h - 1) + 1)) / stride_h + 1;

  const int output_w = (width + 2 * pad_w -
    (dilation_w * (kernel_w - 1) + 1)) / stride_w + 1;
    
  const int channel_size = height * width;


  std::cout << " output_h " << output_h << std::endl;
  std::cout << " output_w " << output_w << std::endl;

  for (int channel = channels; channel--; data_im += channel_size) 
  {
    for (int kernel_row = 0; kernel_row < kernel_h; kernel_row++) 
    {
      for (int kernel_col = 0; kernel_col < kernel_w; kernel_col++) 
      {
        int input_row = -pad_h + kernel_row * dilation_h;
        std::cout << " input_row " << input_row << std::endl;
        for (int output_rows = output_h; output_rows; output_rows--) 
        {
          if (!is_a_ge_zero_and_a_lt_b(input_row, height)) 
          {
            for (int output_cols = output_w; output_cols; output_cols--) 
            {
              *(data_col++) = 0;
              std::cout << " data_col " << data_col << std::endl;
            }
          } 
          else 
          {
            int input_col = -pad_w + kernel_col * dilation_w;
            for (int output_col = output_w; output_col; output_col--)
            {
              if (is_a_ge_zero_and_a_lt_b(input_col, width)) 
              {
                *(data_col++) = data_im[input_row * width + input_col];
                std::cout << " data_col " << data_col << std::endl;
              } 
              else 
              {
                *(data_col++) = 0;
                std::cout << " data_col " << data_col << std::endl;
              }
              input_col += stride_w;
            }
          }
          input_row += stride_h;
        }
      }
    }
  }
}

template void im2col_cpu<float>(const float* data_im, const int channels,
const int height, const int width, const int kernel_h, const int kernel_w,
const int pad_h, const int pad_w, const int stride_h,
const int stride_w, const int dilation_h, const int dilation_w,
float* data_col);

int main()
{
 
    const int height = 4;
    const int width = 4;
    const int kernel_h = 3;
    const int kernel_w = 3;
    const int pad_h = 0;
    const int pad_w = 0;
    const int stride_h = 1;
    const int stride_w = 1;
    const int dilation_h = 0;
    const int dilation_w = 0;
    const int channels = 1;

    std::vector<float> data_im = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    std::vector<float> data_col;

    im2col_cpu(data_im.data(), channels,
    height, width, kernel_h, kernel_w,
    pad_h, pad_w,
    stride_h, stride_w,
    dilation_h, dilation_w,
    data_col.data());


}

