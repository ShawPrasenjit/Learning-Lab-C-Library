#include "llab.h"

/* This function computes the feed forwad of a feature map using the previous convolutional layer
 * 
 * Input:
 *             @ float* input:= a tensor of input of 3 dimensions: channels, rows and cols
 *                              number of feature maps of this layer = number of channels
 *                              dimensions: channels*input_i*input_j
 *             @ float* kernel:= is a tensor of weights used to compute the feature map using the previous convolutional layer
 *                               dimensions: channels*kernel_i*kernel_j
 *             @ int input_i := the number of rows of each feature map of the previous layer (input)
 *             @ int input_j:= the number of columns of each feature map of the previous layer (input)
 *             @ int kernel_i:= the number of rows of each channel of the kernel
 *             @ int kernel_j:= the number of columns of each channel of the kernel
 *             @ float bias:= the bias of the feature map of the current layer
 *             @ int channels:= the depth of the input and the kernel
 *             @ float* output:= the current feature map computed using the input, kernel and bias
 *                               dimensions: ((input_i-kernel_i)/stride + 1 +2*padding)*((input_j-kernel_j)/stride + 1 +2*padding)
 *             @ int stride:= the stride used by the kernel on the feature maps of the inputs
 *             @ int padding:= the optional padding added to the output
 * */
void convolutional_feed_forward(float* input, float* kernel, int input_i, int input_j, int kernel_i, int kernel_j, float bias, int channels, float* output, int stride, int padding){
    int oi,oj,i,j,c;
    int output_i = (input_i-kernel_i)/stride + 1 + 2*padding;
    int output_j = (input_j-kernel_j)/stride + 1 + 2*padding;
    for(oi = padding; oi < output_i-padding; oi++){
        for(oj = padding; oj < output_j-padding; oj++){
            for(c = 0; c < channels; c++){
                for(i = 0; i < kernel_i; i++){
                    for(j = 0; j < kernel_j; j++){
                        output[oi*output_j+oj] += kernel[c*kernel_i*kernel_j + i*kernel_j + j]*input[c*input_i*input_j + i*input_j + j+(oj-padding)*stride+(oi-padding)*stride*input_j];
                    }
                }
            }
            output[oi*output_j+oj] += bias;    
        }
    }
}

/* This function computes the errors using the backpropagation
 * 
 * Input:
 *             @ float* input:= a tensor of input of 3 dimensions: channels, rows and cols
 *                              number of feature maps of this layer = number of channels
 *                              dimensions: channels*input_i*input_j
 *             @ float* kernel:= is a tensor of weights used to compute the feature map using the previous convolutional layer
 *                               dimensions: channels*kernel_i*kernel_j
 *             @ int input_i := the number of rows of each feature map of the previous layer (input)
 *             @ int input_j:= the number of columns of each feature map of the previous layer (input)
 *             @ int kernel_i:= the number of rows of each channel of the kernel
 *             @ int kernel_j:= the number of columns of each channel of the kernel
 *             @ float bias:= the bias of the feature map of the current layer
 *             @ int channels:= the depth of the input and the kernel
 *             @ float* output_error:= the current feature map of the errors
 *                               dimensions: ((input_i-kernel_i)/stride + 1 +2*padding)*((input_j-kernel_j)/stride + 1 +2*padding)
 *             @ float* input_error:= the error of the previous layer computed using the kernel and the current output_error
 *                                    dimensions: channels*input_i*input_j
 *             @ float* kernel_error:= the error of the weights computed using the input and the current output_error
 *                                    dimensions: channels*kernel_i*kernel_j
 *             @ float* bias_error:= the error of the bias
 *                                   dimensions: 1
 *             @ int stride:= the stride used by the kernel on the feature maps of the inputs
 *             @ int padding:= the optional padding added to the output
 * */
void convolutional_back_prop(float* input, float* kernel, int input_i, int input_j, int kernel_i, int kernel_j, float bias, int channels, float* output_error,float* input_error, float* kernel_error, float* bias_error, int stride, int padding){
    int oi,oj,i,j,c;
    int output_i = (input_i-kernel_i)/stride + 1 + 2*padding;
    int output_j = (input_j-kernel_j)/stride + 1 + 2*padding;
    for(oi = padding; oi < output_i-padding; oi++){
        for(oj = padding; oj < output_j-padding; oj++){
            for(c = 0; c < channels; c++){
                for(i = 0; i < kernel_i; i++){
                    for(j = 0; j < kernel_j; j++){
                        kernel_error[c*kernel_i*kernel_j + i*kernel_j + j] += output_error[oi*output_j+oj]*input[c*input_i*input_j + i*input_j + j+(oj-padding)*stride+(oi-padding)*stride*input_j];
                        input_error[c*input_i*input_j + i*input_j + j+(oj-padding)*stride+(oi-padding)*stride*input_j] += kernel[c*kernel_i*kernel_j + i*kernel_j + j]*output_error[oi*output_j+oj];
                    }
                }
            }
            (*bias_error) += output_error[oi*output_j+oj];
        }
    }
}

/* This function apply the 2D max-pooling to a covolutional layer
 * 
 * Input:
 *             @ float* input:= a feature map to which the pooling is applied
 *                              dimensions: input_i*input_j
 *             @ float* output:= the output computed after applying the pooling to the input
 *                               dimensions: ((input_i-sub_pool_i)/stride + 1 + 2*padding)*((input_j-sub_pool_j)/stride + 1 + 2*padding)
 *             @ int input_i:= the rows of the feature map input
 *             @ int input_j:= the number of columns of the feature map output
 *             @ int sub_pool_i:= the number of rows used for each pooling iteration
 *             @ int sub_pool_j:= the number of columns used for each pooling iteration
 *             @ int stride:= the stride used to pool
 *             @ int padding:= the optional padding added to the output
 * */
void max_pooling_feed_forward(float* input, float* output, int input_i, int input_j, int sub_pool_i, int sub_pool_j, int stride, int padding){
    int i,j,k1,k2;
    int output_i = (input_i-sub_pool_i)/stride + 1 + 2*padding;
    int output_j = (input_j-sub_pool_j)/stride + 1 + 2*padding;
    float max = -9999;
    
    for(i = 0; i < output_i - 2*padding; i++){
        for(j = 0; j < output_j - 2*padding; j++){
            for(k1 = 0; k1 < sub_pool_i; k1++){
                for(k2 = 0; k2 < sub_pool_j; k2++){                   
                    if(input[input_j*(i*stride+k1) + j*stride + k2] > max)
                        max = input[input_j*(i*stride+k1) + j*stride + k2];
                }
            }
            output[(padding+i)*output_j+padding+j] = max;
            max = -99999;        
        }
    }
    
     
}

/* This function computed the error for a max-pool layer
 * 
 * Input:
 *             @ float* input:= a feature map to which the pooling is applied
 *                              dimensions: input_i*input_j
 *             @ float* output_error:= the output_error used to compute the input error
 *                               dimensions: ((input_i-sub_pool_i)/stride + 1 + 2*padding)*((input_j-sub_pool_j)/stride + 1 + 2*padding)
 *             @ int input_i:= the rows of the feature map input
 *             @ int input_j:= the number of columns of the feature map output
 *             @ int sub_pool_i:= the number of rows used for each pooling iteration
 *             @ int sub_pool_j:= the number of columns used for each pooling iteration
 *             @ int stride:= the stride used to pool
 *             @ int padding:= the optional padding added to the output
 *             @ float input_error := the error computed using the output_error
 *                                    dimensions: input_i*input_j
 * */
void max_pooling_back_prop(float* input, float* output_error, int input_i, int input_j, int sub_pool_i, int sub_pool_j, int stride, int padding, float* input_error){
    int i,j,k1,k2, index1, index2;
    int output_i = (input_i-sub_pool_i)/stride + 1 + 2*padding;
    int output_j = (input_j-sub_pool_j)/stride + 1 + 2*padding;
    float max = -9999;
    
    for(i = 0; i < output_i - 2*padding; i++){
        for(j = 0; j < output_j - 2*padding; j++){
            for(k1 = 0; k1 < sub_pool_i; k1++){
                for(k2 = 0; k2 < sub_pool_j; k2++){
                    
                    if(input[input_j*(i*stride+k1) + j*stride + k2] > max){
                        max = input[input_j*(i*stride+k1) + j*stride + k2];
                        index1 = k1;
                        index2 = k2;
                        
                    }
                
                }
            }
            
            for(k1 = 0; k1 < sub_pool_i; k1++){
                for(k2 = 0; k2 < sub_pool_j; k2++){
                    
                    if(index1 == k1 && index2 == k2)
                        input_error[input_j*(i*stride+k1) + j*stride + k2] = output_error[(padding+i)*output_j+padding+j];
                    
                    else
                        input_error[input_j*(i*stride+k1) + j*stride + k2] = 0;
                        
                
                }
            }
            max = -99999;        
        }
    } 
}

/* This function apply the 2D avarage-pooling to a covolutional layer
 * 
 * Input:
 *             @ float* input:= a feature map to which the pooling is applied
 *                              dimensions: input_i*input_j
 *             @ float* output:= the output computed after applying the pooling to the input
 *                               dimensions: ((input_i-sub_pool_i)/stride + 1 + 2*padding)*((input_j-sub_pool_j)/stride + 1 + 2*padding)
 *             @ int input_i:= the rows of the feature map input
 *             @ int input_j:= the number of columns of the feature map output
 *             @ int sub_pool_i:= the number of rows used for each pooling iteration
 *             @ int sub_pool_j:= the number of columns used for each pooling iteration
 *             @ int stride:= the stride used to pool
 *             @ int padding:= the optional padding added to the output
 * */
void avarage_pooling_feed_forward(float* input, float* output, int input_i, int input_j, int sub_pool_i, int sub_pool_j, int stride, int padding){
    int i,j,k1,k2;
    int output_i = (input_i-sub_pool_i)/stride + 1 + 2*padding;
    int output_j = (input_j-sub_pool_j)/stride + 1 + 2*padding;
    float sum = 0;
    
    for(i = 0; i < output_i - 2*padding; i++){
        for(j = 0; j < output_j - 2*padding; j++){
            for(k1 = 0; k1 < sub_pool_i; k1++){
                for(k2 = 0; k2 < sub_pool_j; k2++){
                    sum += input[input_j*(i*stride+k1) + j*stride + k2];
                }
            }
            output[(padding+i)*output_j+padding+j] = sum/(sub_pool_i*sub_pool_j);
            sum = 0;        
        }
    }
}

/* This function computed the error for an avarage-pool layer
 * 
 * Input:
 *            @ float input_error := the error computed using the output_error
 *                                    dimensions: input_i*input_j
 *             @ float* output_error:= the output_error used to compute the input error
 *                               dimensions: ((input_i-sub_pool_i)/stride + 1 + 2*padding)*((input_j-sub_pool_j)/stride + 1 + 2*padding)
 *             @ int input_i:= the rows of the feature map input
 *             @ int input_j:= the number of columns of the feature map output
 *             @ int sub_pool_i:= the number of rows used for each pooling iteration
 *             @ int sub_pool_j:= the number of columns used for each pooling iteration
 *             @ int stride:= the stride used to pool
 *             @ int padding:= the optional padding added to the output
 *             
 * */
void avarage_pooling_back_prop(float* input_error, float* output_error, int input_i, int input_j, int sub_pool_i, int sub_pool_j, int stride, int padding){
    int i,j,k1,k2;
    int output_i = (input_i-sub_pool_i)/stride + 1 + 2*padding;
    int output_j = (input_j-sub_pool_j)/stride + 1 + 2*padding;
    
    for(i = 0; i < output_i - 2*padding; i++){
        for(j = 0; j < output_j - 2*padding; j++){
            for(k1 = 0; k1 < sub_pool_i; k1++){
                for(k2 = 0; k2 < sub_pool_j; k2++){
                    input_error[input_j*(i*stride+k1) + j*stride + k2] = ((float)1/(sub_pool_i*sub_pool_j))*output_error[(padding+i)*output_j+padding+j];
                }
            }
        }
    }
}
