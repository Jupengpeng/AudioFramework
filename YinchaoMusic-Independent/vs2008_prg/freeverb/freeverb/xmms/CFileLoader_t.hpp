class _FV3_(CFileLoader)
{
 public:
  _FV3_(CFileLoader)(){ rawStream = srcStream = NULL; };
  virtual _FV3_(~CFileLoader)(){ dealloc(); };
  // stretch x1=1.0f limit 100%=100
  int load(const char * filename, double fs, double stretch, double limit, long src_type)
  {
    if(limit <= 0) limit = 100;
    SndfileHandle sndHandle(filename);
    if(sndHandle.frames() <= 0) return -1;
    //double second = (double)sndHandle.frames()/(double)sndHandle.samplerate();
    double ratio = (double)stretch*(double)fs/(double)sndHandle.samplerate();
    dealloc();
    try
      {
	rawStream = new _fv3_float_t[((int)sndHandle.frames())*sndHandle.channels()];
	srcStream = new _fv3_float_t[sndHandle.frames()*sndHandle.channels()*((int)ratio+1)];
      }
    catch(std::bad_alloc){ dealloc(); return -2; }
    sf_count_t rcount = sndHandle.readf(rawStream, sndHandle.frames());
    if(rcount != sndHandle.frames()){ dealloc(); return -3; }
    int srcOutputFrames;
    _fv3_float_t * irsStream = NULL;
    if(fs != sndHandle.samplerate()||stretch != 1.0f)
      {
	int srcerr;
	_FV3_(SRC_DATA) src_data;
	src_data.src_ratio = ratio;
	src_data.data_in = rawStream;
	src_data.input_frames = sndHandle.frames();
	src_data.data_out = srcStream;
	src_data.output_frames = src_data.input_frames*((int)ratio+1);
	src_data.end_of_input = 1;
	srcerr = _FV3_(src_simple)(&src_data, src_type, sndHandle.channels());
	if(srcerr != 0){ dealloc(); errstring = src_strerror(srcerr); return -4; }
	srcOutputFrames = src_data.output_frames_gen;
	irsStream = srcStream;
      }
    else
      {
	srcOutputFrames = sndHandle.frames();
	irsStream = rawStream;
      }
    long copyFrames = (long)((double)srcOutputFrames*limit/100.0f);
    try
      {
	out.alloc(copyFrames, sndHandle.channels());
      }
    catch(std::bad_alloc){ dealloc(); return -5; }
    splitChannelsS(sndHandle.channels(), copyFrames, irsStream, out.getArray());
    dealloc();
    return 0;
  }
  const char * errstr(){ return errstring.c_str(); };
  _FV3_(slot) out;
 private:
  void dealloc()
    {
      delete[] rawStream;
      delete[] srcStream;
      rawStream = srcStream = NULL;
    }
  _fv3_float_t *rawStream, *srcStream;
  std::string errstring;
};
