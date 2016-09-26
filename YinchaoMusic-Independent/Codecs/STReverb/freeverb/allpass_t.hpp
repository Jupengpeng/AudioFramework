class _FV3_(allpass)
{
 public:
  _FV3_(allpass)();
  virtual _FV3_(~allpass)();
  void setsize(long size) /*throw(std::bad_alloc)*/;
  long getsize();

  inline _fv3_float_t getlast()
  {
    return buffer[bufidx];
  }

  inline _fv3_float_t get_z(long index)
  {
    if(index > bufsize) index = bufsize;
    if(index <= 0) index = 1;
    long readpoint = bufidx - index;
    if(readpoint < 0) readpoint += bufsize;
    return buffer[readpoint];
  }

  /**
   * normal allpass filter.
   * @param[in] input signal.
   */  
  inline _fv3_float_t process(_fv3_float_t input)
  {
    if(bufsize == 0) return input;
    _fv3_float_t bufout = buffer[bufidx];
    UNDENORMAL(bufout);
    buffer[bufidx] = input + bufout * feedback;
    _fv3_float_t output = bufout - buffer[bufidx] * feedback;
    bufidx ++; if(bufidx >= bufsize) bufidx = 0;
    return output;
  }

  /**
   * original freeverb version of allpass filter.
   * @param[in] input signal.
   */
  inline _fv3_float_t process_ov(_fv3_float_t input)
  {
    if(bufsize == 0) return input;
    _fv3_float_t bufout = buffer[bufidx];
    UNDENORMAL(bufout);
    buffer[bufidx] = input + (bufout * feedback);
    bufidx ++; if(bufidx >= bufsize) bufidx = 0;
    return (bufout - input);
  }
  
  void mute();
  void setfeedback(_fv3_float_t val);
  _fv3_float_t getfeedback();
 
 private:
  _FV3_(allpass)(const _FV3_(allpass)& x);
  _FV3_(allpass)& operator=(const _FV3_(allpass)& x);
  _fv3_float_t feedback, *buffer;
  long bufsize, bufidx;
};

class _FV3_(allpassm)
{
 public:
  _FV3_(allpassm)();
  virtual _FV3_(~allpassm)();
  void setsize(long size) /*throw(std::bad_alloc)*/;
  void setsize(long size, long modsize) /*throw(std::bad_alloc)*/;
  long getsize();
  long getdelaysize();
  long getmodulationsize();
  void mute();
  void setfeedback(_fv3_float_t val);
  _fv3_float_t getfeedback();
  void set_90degfq(_fv3_float_t fc, _fv3_float_t fs);

  inline _fv3_float_t process(_fv3_float_t input)
  {
    return process(input, 0);
  }

  inline _fv3_float_t process(_fv3_float_t input, _fv3_float_t modulation)
  {
    if(bufsize == 0) return input;
    if(modulation < -1) modulation = -1; if(modulation > 1) modulation = 1;
    modulation += 1.;
    modulation *= (_fv3_float_t)modulationsize;
    _fv3_float_t floor_mod = std::floor(modulation); // >= 0
    _fv3_float_t frac = modulation - floor_mod; // >= 0
    
    long readidx_a = readidx - (long)floor_mod;
    if(readidx_a < 0) readidx_a += bufsize;
    long readidx_b = readidx_a - 1;
    if(readidx_b < 0) readidx_b += bufsize;
    
    _fv3_float_t temp = buffer[readidx_b] + buffer[readidx_a]*(1-frac) - (1-frac)*z_1;
    UNDENORMAL(temp);
    z_1 = temp;
    
    readidx ++; if(readidx >= bufsize) readidx = 0;

    buffer[writeidx] = input + temp * feedback;
    _fv3_float_t output = temp - buffer[writeidx] * feedback;
    writeidx ++; if(writeidx >= bufsize) writeidx = 0;
    
    return output;
  }
 
 private:
  _FV3_(allpassm)(const _FV3_(allpassm)& x);
  _FV3_(allpassm)& operator=(const _FV3_(allpassm)& x);
  _fv3_float_t feedback, *buffer, z_1;
  long bufsize, readidx, writeidx, delaysize, modulationsize;
};
