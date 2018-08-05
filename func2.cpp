#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

template <typename Dtype>
void im2col_cpu(const Dtype* data_im, const int channels,
  const int height, const int width, const int kernel_h, const int kernel_w,
  const int pad_h, const int pad_w,
  const int stride_h, const int stride_w,
  const int dilation_h, const int dilation_w,
  Dtype* data_col) 
{    
    
  int dil_kernel_h = (kernel_h - 1) * dilation_h + 1;
  int dil_kernel_w = (kernel_w - 1) * dilation_w + 1;
  int height_col = (height + 2 * pad_h - dil_kernel_h) / stride_h + 1;
  int width_col = (width + 2 * pad_w - dil_kernel_w) / stride_w + 1;
  int channels_col = channels * kernel_h * kernel_w;
  
  std::cout<<dil_kernel_h<<" dil_kernel_h"<<std::endl;
  std::cout<<dil_kernel_w<<" dil_kernel_w"<<std::endl;
  std::cout<<height_col<<" height_col"<<std::endl;
  std::cout<<width_col <<" width_col"<<std::endl;
  
  #ifdef _OPENMP
  #pragma omp parallel for
  #endif
  for (int c = 0; c < channels_col; ++c) 
  {
    int w_offset = c % kernel_w;
    int h_offset = (c / kernel_w) % kernel_h;
    int c_im = c / kernel_h / kernel_w;
    
    const int hc0 = h_offset * dilation_h - pad_h;
    const int wc0 = w_offset * dilation_w - pad_w;
    
   
    
    
    for (int h = 0; h < height_col; ++h) 
    {
      int h_pad = h * stride_h + hc0;
      const int row_offset = (c * height_col + h) * width_col;
      const int srow_offset = (c_im * height + h_pad) * width;
      
      for (int w = 0; w < width_col; ++w) 
      {
        int w_pad = w * stride_w + wc0;
        
        
        
        if ((((unsigned)h_pad) < ((unsigned)height)) && (((unsigned)w_pad) < ((unsigned)width)))
        {
          
          data_col[row_offset + w] = data_im[srow_offset + w_pad];
          std::cout<< " value "<<data_col[row_offset + w]<<std::endl;  
           //std::cout<< " data_col " << *data_col <<std::endl;  
          
        }
        else 
        {
          data_col[row_offset + w] = 0.;
        }
      }
    }  
  }
}


// Explicit instantiation
template void im2col_cpu<float>(const float* data_im, const int channels,
  const int height, const int width, const int kernel_h, const int kernel_w,
  const int pad_h, const int pad_w,
  const int stride_h, const int stride_w,
  const int dilation_h, const int dilation_w,
  float* data_col) ;




int main()
{
 
  const int height = 4;
  const int width = 4;
  const int channels = 1;
  
  const int kernel_h = 3;
  const int kernel_w = 3;
  const int filter_count = 1;
  const int pad_h = 0;
  const int pad_w = 0;
  const int stride_h = 1;
  const int stride_w = 1;
  const int dilation_h = 1;
  const int dilation_w = 1;


  std::vector<float> input_data = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  
  
  //std::vector<float> filter_data = {1,2,3,4,5,6,7,8,9};
  float filter_data[kernel_h*kernel_w] = {1,2,3,4,5,6,7,8,9};
  //std::vector<float> output_data;
  
  std::vector<float> output_data(36);

  
  im2col_cpu<float>(input_data.data(), channels, height, width, kernel_h, kernel_w,
                    pad_h, pad_w, stride_h, stride_w, dilation_h, dilation_w,
                    output_data.data());


}
