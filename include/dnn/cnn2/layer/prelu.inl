
static int PReLU(enum LayerOp op, struct NetWork* net, struct Layer* layer)
{
  const Blob* bottom_data = LBLOB(0)->data;
  Blob* top_blob = net->blobs[layer->blobs[1]].data;
  Blob* bottom_top_blob = top_blob;
  int w = bottom_data->w;
  int h = bottom_data->h;
  int channels = Blob_channels(bottom_data);
  int size = w * h;
  int num_slope = LARG(data_size);
  const float* slope_data_ptr = Blob_datap(LBLOB(2)->data);
  
  if (bottom_data==top_blob) {
#pragma omp parallel for
    for (int q = 0; q < channels; q++) {
      float* ptr = Blob_channel_p(bottom_top_blob, q);
      float slope = num_slope > 1 ? slope_data_ptr[q] : slope_data_ptr[0];
      
      for (int i = 0; i < size; i++) {
        if (ptr[i] < 0) {
          ptr[i] *= slope;
        }
      }
    }
  } else {
    Blob_create(top_blob, w, h, channels);
    
    if (imempty(top_blob)) {
      return -100;
    }
    
    
#pragma omp parallel for
    for (int q = 0; q < channels; q++) {
      const float* ptr = Blob_channel_p(bottom_data, q);
      float* outptr = Blob_channel_p(top_blob, q);
      float slope = num_slope > 1 ? slope_data_ptr[q] : slope_data_ptr[0];
      
      for (int i = 0; i < size; i++) {
        if (ptr[i] < 0) {
          outptr[i] = ptr[i] * slope;
        }
        else {
          outptr[i] = ptr[i];
        }
      }
    }
  }
  return 0;
}