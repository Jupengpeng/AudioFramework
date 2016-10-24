class _FV3_(biquad)
{
 public:
  _FV3_(biquad)();
  virtual _FV3_(~biquad)();
  void printconfig();
  void mute();
  _fv3_float_t get_A1(){return a1;}
  _fv3_float_t get_A2(){return a2;}
  _fv3_float_t get_B0(){return b0;}
  _fv3_float_t get_B1(){return b1;}
  _fv3_float_t get_B2(){return b2;}
  void set_A1(_fv3_float_t v){a1=v;}
  void set_A2(_fv3_float_t v){a2=v;}
  void set_B0(_fv3_float_t v){b0=v;}
  void set_B1(_fv3_float_t v){b1=v;}
  void set_B2(_fv3_float_t v){b2=v;}
  
  void setCoefficients(_fv3_float_t _b0, _fv3_float_t _b1, _fv3_float_t _b2, _fv3_float_t _a1, _fv3_float_t _a2);
  void setLPF(_fv3_float_t fc, _fv3_float_t bw, _fv3_float_t fs);
  void setHPF(_fv3_float_t fc, _fv3_float_t bw, _fv3_float_t fs);
  void setEQP(_fv3_float_t fc, _fv3_float_t gain, _fv3_float_t bw, _fv3_float_t fs);
  void setLSF(_fv3_float_t fc, _fv3_float_t gain, _fv3_float_t slope, _fv3_float_t fs);
  void setHSF(_fv3_float_t fc, _fv3_float_t gain, _fv3_float_t slope, _fv3_float_t fs);

  inline _fv3_float_t process(_fv3_float_t input)
  {
    return this->processd1(input);
  }
  
  // Direct form I
  inline _fv3_float_t processd1(_fv3_float_t input)
  {
    _fv3_float_t i0 = input;
    input *= b0;
    input += b1 * i1 + b2 * i2;
    input -= a1 * o1 + a2 * o2 ;
    UNDENORMAL(input);
    i2 = i1; i1 = i0;
    o2 = o1; o1 = input;
    return input;
  }

  // Direct form II
  inline _fv3_float_t processd2(_fv3_float_t input)
  {
    input -= a1 * t1 + a2 * t2 ;
    t0 = input; input *= b0;
    input += b1 * t1 + b2 * t2;
    UNDENORMAL(input);
    t2 = t1; t1 = t0;
    return input;
  }

 private:
  _FV3_(biquad)(const _FV3_(biquad)& x);
  _FV3_(biquad)& operator=(const _FV3_(biquad)& x);
  _fv3_float_t a1, a2, b0, b1, b2;
  _fv3_float_t i1, i2, o1, o2;
  _fv3_float_t t0, t1, t2;
};
