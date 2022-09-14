///////////////////////////////////////////////////////////
//// NOTE(Elias): Float Constant Functions

internal F32 
inf_F32(void)
{
  union{ F32 f; U32 u; } r;
  r.u = 0x7f800000;
  return(r.f);
}
internal F32 
neg_inf_F32(void)
{
  union{ F32 f; U32 u; } r;
  r.u = 0xff800000;
  return(r.f);
}

internal F64 
inf_F64(void)
{
  union{ F64 f; U64 u; } r;
  r.u = 0x7ff00000ff800000;
  return(r.f);
}

internal F64 
neg_inf_F64(void)
{
  union{ F64 f; U64 u; } r;
  r.u = 0xfff00000ff800000;
  return(r.f);
}

///////////////////////////////////////////////////////////
//// NOTE(Elias): Math functions 

internal F32 
abs_F32(F32 x)
{
  union{ F32 f; U32 u; } r;
  r.f = x;
  r.u = r.u & 0x7fffffff;
  return(r.f);
}

internal F64 
abs_F64(F64 x)
{
  union{ F32 f; U32 u; } r;
  r.f = x;
  r.u = r.u & 0x7fffffffffffffff;
  return(r.f);
} 

