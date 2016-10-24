class _FV3_(comb)
{
public:
  _FV3_(comb)();
  virtual _FV3_(~comb)();
  void setsize(long size) throw(std::bad_alloc);
  long getsize();

  inline _fv3_float_t process(_fv3_float_t input)
  {
    if(bufsize == 0) return input;
    _fv3_float_t output = buffer[bufidx];
    UNDENORMAL(output);
    filterstore = (output * damp2) + (filterstore * damp1);
    UNDENORMAL(filterstore);
    buffer[bufidx] = input + (filterstore * feedback);
    bufidx ++; if(bufidx >= bufsize) bufidx = 0;
    return output;
  }

  void mute();
  void setdamp(_fv3_float_t val);
  _fv3_float_t  getdamp();
  void  setfeedback(_fv3_float_t val);
  _fv3_float_t  getfeedback();

private:
  _FV3_(comb)(const _FV3_(comb)& x);
  _FV3_(comb)& operator=(const _FV3_(comb)& x);
  _fv3_float_t *buffer, feedback, filterstore, damp1, damp2;
  long bufsize, bufidx;
};

class _FV3_(combm)
{
public:
  _FV3_(combm)();
  virtual _FV3_(~combm)();
  void setsize(long size) throw(std::bad_alloc);
  void setsize(long size, long modsize) throw(std::bad_alloc);
  long getsize();
  long getdelaysize();
  long getmodulationsize();
  void mute();
  void setdamp(_fv3_float_t val);
  _fv3_float_t	getdamp();
  void	setfeedback(_fv3_float_t val);
  _fv3_float_t	getfeedback();
  
  inline _fv3_float_t process(_fv3_float_t input)
  {
    if(bufsize == 0) return input;
    _fv3_float_t output = buffer[readidx];
    UNDENORMAL(output);
    filterstore = (output * damp2) + (filterstore * damp1);
    UNDENORMAL(filterstore);
    buffer[readidx] = input + (filterstore * feedback);
    readidx ++; if(readidx >= bufsize) readidx = 0;
    return output;
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

    filterstore = temp * damp2 + filterstore * damp1;
    UNDENORMAL(filterstore);

    buffer[writeidx] = input + filterstore * feedback;
    writeidx ++; if(writeidx >= bufsize) writeidx = 0;
    
    return temp;
  }

private:
  _FV3_(combm)(const _FV3_(combm)& x);
  _FV3_(combm)& operator=(const _FV3_(combm)& x);
  _fv3_float_t *buffer, feedback, filterstore, damp1, damp2, z_1;
  long bufsize, readidx, writeidx, delaysize, modulationsize;
};
