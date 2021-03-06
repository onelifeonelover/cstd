
struct PReLU_arm : public PReLU
{
public:
  virtual int forward(const Blob* prev, Blob* next) const;

  virtual int forward_inplace(Blob* bottom_top_blob) const;
};


DEFINE_LAYER_CREATOR(PReLU_arm)

int PReLU_forward(const Blob* prev, Blob* next) const
{
  int w = prevSize.w;
  int h = prevSize.h;
  int channels = prevSize.c;
  int size = w * h;

  next->create(w, h, channels);

  if (next->empty()) {
    return -100;
  }

  const float* slope_data_ptr = slope_data;

  #pragma omp parallel for

  for (q = 0; q < channels; q++) {
    const float* ptr = prevData + q * Data_2DSize(prevSize);
    float* outptr = nextData + q * Data_2DSize(nextSize);
    float slope = num_slope > 1 ? slope_data_ptr[q] : slope_data_ptr[0];

#if __ARM_NEON
    int nn = size >> 2;
    int remain = size - (nn << 2);
#else
    int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
#if __aarch64__
    float32x4_t _zero = vdupq_n_f32(0.f);
    float32x4_t _slope = vdupq_n_f32(slope);

    for (; nn > 0; nn--) {
      float32x4_t _p = vld1q_f32(ptr);
      uint32x4_t _lemask = vcleq_f32(_p, _zero);
      float32x4_t _ps = vmulq_f32(_p, _slope);
      float32x4_t _outp = vbslq_f32(_lemask, _ps, _p);
      vst1q_f32(outptr, _outp);

      ptr += 4;
      outptr += 4;
    }

#else

    if (nn > 0) {
      asm volatile(
          "veor       q1, q0, q0          \n"
          "vdup.f32   q2, %6              \n"
          "0:                             \n"
          "pld        [%1, #128]          \n"
          "vld1.f32   {d0-d1}, [%1 :128]  \n"
          "vcle.f32   q3, q0, q1          \n"
          "vmul.f32   q4, q0, q2          \n"
          "vbit.32    q0, q4, q3          \n"
          "subs       %0, #1              \n"
          "vst1.f32   {d0-d1}, [%2 :128]! \n"
          "bne        0b                  \n"
          : "=r"(nn),     // %0
          "=r"(ptr),    // %1
          "=r"(outptr)  // %2
          : "0"(nn),
          "1"(ptr),
          "2"(outptr),
          "r"(slope)    // %6
          : "cc", "memory", "q0", "q1", "q2", "q3", "q4"
      );
    }

#endif // __aarch64__
#endif // __ARM_NEON

    for (; remain > 0; remain--) {
      if (*ptr < 0) {
        *outptr = *ptr * slope;
      }
      else {
        *outptr = *ptr;
      }

      ptr++;
      outptr++;
    }
  }

  return 0;
}

int PReLU_forward_inplace(Blob* bottom_top_blob) const
{
  int w = bottom_top_blob->w;
  int h = bottom_top_blob->h;
  int channels = Blob_channels(bottom_top_blob);
  int size = w * h;

  const float* slope_data_ptr = slope_data;

  #pragma omp parallel for

  for (q = 0; q < channels; q++) {
    float* ptr = Blob_channel_p(bottom_top_blob, q);
    float slope = num_slope > 1 ? slope_data_ptr[q] : slope_data_ptr[0];

#if __ARM_NEON
    int nn = size >> 2;
    int remain = size - (nn << 2);
#else
    int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
#if __aarch64__
    float32x4_t _zero = vdupq_n_f32(0.f);
    float32x4_t _slope = vdupq_n_f32(slope);

    for (; nn > 0; nn--) {
      float32x4_t _p = vld1q_f32(ptr);
      uint32x4_t _lemask = vcleq_f32(_p, _zero);
      float32x4_t _ps = vmulq_f32(_p, _slope);
      _p = vbslq_f32(_lemask, _ps, _p);
      vst1q_f32(ptr, _p);

      ptr += 4;
    }

#else

    if (nn > 0) {
      asm volatile(
          "veor       q1, q0, q0          \n"
          "vdup.f32   q2, %4              \n"
          "0:                             \n"
          "pld        [%1, #128]          \n"
          "vld1.f32   {d0-d1}, [%1 :128]  \n"
          "vcle.f32   q3, q0, q1          \n"
          "vmul.f32   q4, q0, q2          \n"
          "vbit.32    q0, q4, q3          \n"
          "subs       %0, #1              \n"
          "vst1.f32   {d0-d1}, [%1 :128]! \n"
          "bne        0b                  \n"
          : "=r"(nn),     // %0
          "=r"(ptr)     // %1
          : "0"(nn),
          "1"(ptr),
          "r"(slope)    // %4
          : "cc", "memory", "q0", "q1", "q2", "q3", "q4"
      );
    }

#endif // __aarch64__
#endif // __ARM_NEON

    for (; remain > 0; remain--) {
      if (*ptr < 0) {
        *ptr *= slope;
      }

      ptr++;
    }
  }

  return 0;
}

} // namespace ncnn
