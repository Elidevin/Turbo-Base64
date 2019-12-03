/**
Copyright (c) 2016-2019, Powturbo
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
//------------- TurboBase64 : Base64 encoding ----------------------
#include "conf.h"
#include "turbob64.h"
#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BSWAP32(a) a 
  #else
#define BSWAP32(a) bswap32(a)
  #endif  

unsigned turbob64len(unsigned n) { return TURBOB64LEN(n); }

//--------------------------------------------------------------
#define LU32(_u_) (lut1[(_u_>> 8) & 0x3f] << 24 |\
                   lut1[(_u_>>14) & 0x3f] << 16 |\
                   lut1[(_u_>>20) & 0x3f] <<  8 |\
                   lut1[(_u_>>26) & 0x3f])

#define ETAIL() \
  if(outlen - (op-out) > 4)\
    for(; op < out+(outlen-4); op += 4, ip+= 3) { unsigned _u = BSWAP32(ctou32(ip)); ctou32(op) = LU32(_u); }\
  unsigned _l = inlen - (ip-in); printf("L=%d ", _l);\
  if(_l == 3) { unsigned _u = BSWAP32(ip[0] | ip[1]<<8 | ip[2]<<16); ctou32(op) = LU32(_u); op+=4; ip+=3; }\
  else if(_l) { *op++ = lut1[(ip[0]>>2)&0x3f];\
    if(_l == 2) *op++ = lut1[(ip[0] & 0x3) << 4 | (ip[1] & 0xf0) >> 4],\
                *op++ = lut1[(ip[1] & 0xf) << 2];\
    else        *op++ = lut1[(ip[0] & 0x3) << 4], *op++ = '=';\
    *op++ = '=';\
  }

//----------------------- small 64 bytes lut encoding ---------------------------------------
#define LI32(_i_) { \
  unsigned _u = BSWAP32(ctou32(ip+_i_*6    ));\
  unsigned _v = BSWAP32(ctou32(ip+_i_*6 + 3));\
  _u = LU32(_u);\
  _v = LU32(_v);\
  ctou32(op+_i_*8    ) = _u;\
  ctou32(op+_i_*8 + 4) = _v;\
}
                              
unsigned tb64senc(const unsigned char *in, unsigned inlen, unsigned char *out) {
  static unsigned char lut1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const  unsigned char *ip    = in;
         unsigned char *op    = out;
         unsigned      outlen = TURBOB64LEN(inlen);

  if(outlen >= 128+4)
    for(; op <= out+(outlen-(128+4)); op += 128, ip += (128/4)*3) {                     // 96->128 bytes
      LI32(0); LI32(1); LI32( 2); LI32( 3); LI32( 4); LI32( 5); LI32( 6); LI32( 7);         
      LI32(8); LI32(9); LI32(10); LI32(11); LI32(12); LI32(13); LI32(14); LI32(15);    PREFETCH(ip,256, 0);
    }
  ETAIL(); 
  return outlen;
}

//---------------------- Fast encoding with 4k LUT ------------------------------------------------------------
static const unsigned short lut2[1<<12] = { 
0x4141,0x4241,0x4341,0x4441,0x4541,0x4641,0x4741,0x4841,0x4941,0x4a41,0x4b41,0x4c41,0x4d41,0x4e41,0x4f41,0x5041,
0x5141,0x5241,0x5341,0x5441,0x5541,0x5641,0x5741,0x5841,0x5941,0x5a41,0x6141,0x6241,0x6341,0x6441,0x6541,0x6641,
0x6741,0x6841,0x6941,0x6a41,0x6b41,0x6c41,0x6d41,0x6e41,0x6f41,0x7041,0x7141,0x7241,0x7341,0x7441,0x7541,0x7641,
0x7741,0x7841,0x7941,0x7a41,0x3041,0x3141,0x3241,0x3341,0x3441,0x3541,0x3641,0x3741,0x3841,0x3941,0x2b41,0x2f41,
0x4142,0x4242,0x4342,0x4442,0x4542,0x4642,0x4742,0x4842,0x4942,0x4a42,0x4b42,0x4c42,0x4d42,0x4e42,0x4f42,0x5042,
0x5142,0x5242,0x5342,0x5442,0x5542,0x5642,0x5742,0x5842,0x5942,0x5a42,0x6142,0x6242,0x6342,0x6442,0x6542,0x6642,
0x6742,0x6842,0x6942,0x6a42,0x6b42,0x6c42,0x6d42,0x6e42,0x6f42,0x7042,0x7142,0x7242,0x7342,0x7442,0x7542,0x7642,
0x7742,0x7842,0x7942,0x7a42,0x3042,0x3142,0x3242,0x3342,0x3442,0x3542,0x3642,0x3742,0x3842,0x3942,0x2b42,0x2f42,
0x4143,0x4243,0x4343,0x4443,0x4543,0x4643,0x4743,0x4843,0x4943,0x4a43,0x4b43,0x4c43,0x4d43,0x4e43,0x4f43,0x5043,
0x5143,0x5243,0x5343,0x5443,0x5543,0x5643,0x5743,0x5843,0x5943,0x5a43,0x6143,0x6243,0x6343,0x6443,0x6543,0x6643,
0x6743,0x6843,0x6943,0x6a43,0x6b43,0x6c43,0x6d43,0x6e43,0x6f43,0x7043,0x7143,0x7243,0x7343,0x7443,0x7543,0x7643,
0x7743,0x7843,0x7943,0x7a43,0x3043,0x3143,0x3243,0x3343,0x3443,0x3543,0x3643,0x3743,0x3843,0x3943,0x2b43,0x2f43,
0x4144,0x4244,0x4344,0x4444,0x4544,0x4644,0x4744,0x4844,0x4944,0x4a44,0x4b44,0x4c44,0x4d44,0x4e44,0x4f44,0x5044,
0x5144,0x5244,0x5344,0x5444,0x5544,0x5644,0x5744,0x5844,0x5944,0x5a44,0x6144,0x6244,0x6344,0x6444,0x6544,0x6644,
0x6744,0x6844,0x6944,0x6a44,0x6b44,0x6c44,0x6d44,0x6e44,0x6f44,0x7044,0x7144,0x7244,0x7344,0x7444,0x7544,0x7644,
0x7744,0x7844,0x7944,0x7a44,0x3044,0x3144,0x3244,0x3344,0x3444,0x3544,0x3644,0x3744,0x3844,0x3944,0x2b44,0x2f44,
0x4145,0x4245,0x4345,0x4445,0x4545,0x4645,0x4745,0x4845,0x4945,0x4a45,0x4b45,0x4c45,0x4d45,0x4e45,0x4f45,0x5045,
0x5145,0x5245,0x5345,0x5445,0x5545,0x5645,0x5745,0x5845,0x5945,0x5a45,0x6145,0x6245,0x6345,0x6445,0x6545,0x6645,
0x6745,0x6845,0x6945,0x6a45,0x6b45,0x6c45,0x6d45,0x6e45,0x6f45,0x7045,0x7145,0x7245,0x7345,0x7445,0x7545,0x7645,
0x7745,0x7845,0x7945,0x7a45,0x3045,0x3145,0x3245,0x3345,0x3445,0x3545,0x3645,0x3745,0x3845,0x3945,0x2b45,0x2f45,
0x4146,0x4246,0x4346,0x4446,0x4546,0x4646,0x4746,0x4846,0x4946,0x4a46,0x4b46,0x4c46,0x4d46,0x4e46,0x4f46,0x5046,
0x5146,0x5246,0x5346,0x5446,0x5546,0x5646,0x5746,0x5846,0x5946,0x5a46,0x6146,0x6246,0x6346,0x6446,0x6546,0x6646,
0x6746,0x6846,0x6946,0x6a46,0x6b46,0x6c46,0x6d46,0x6e46,0x6f46,0x7046,0x7146,0x7246,0x7346,0x7446,0x7546,0x7646,
0x7746,0x7846,0x7946,0x7a46,0x3046,0x3146,0x3246,0x3346,0x3446,0x3546,0x3646,0x3746,0x3846,0x3946,0x2b46,0x2f46,
0x4147,0x4247,0x4347,0x4447,0x4547,0x4647,0x4747,0x4847,0x4947,0x4a47,0x4b47,0x4c47,0x4d47,0x4e47,0x4f47,0x5047,
0x5147,0x5247,0x5347,0x5447,0x5547,0x5647,0x5747,0x5847,0x5947,0x5a47,0x6147,0x6247,0x6347,0x6447,0x6547,0x6647,
0x6747,0x6847,0x6947,0x6a47,0x6b47,0x6c47,0x6d47,0x6e47,0x6f47,0x7047,0x7147,0x7247,0x7347,0x7447,0x7547,0x7647,
0x7747,0x7847,0x7947,0x7a47,0x3047,0x3147,0x3247,0x3347,0x3447,0x3547,0x3647,0x3747,0x3847,0x3947,0x2b47,0x2f47,
0x4148,0x4248,0x4348,0x4448,0x4548,0x4648,0x4748,0x4848,0x4948,0x4a48,0x4b48,0x4c48,0x4d48,0x4e48,0x4f48,0x5048,
0x5148,0x5248,0x5348,0x5448,0x5548,0x5648,0x5748,0x5848,0x5948,0x5a48,0x6148,0x6248,0x6348,0x6448,0x6548,0x6648,
0x6748,0x6848,0x6948,0x6a48,0x6b48,0x6c48,0x6d48,0x6e48,0x6f48,0x7048,0x7148,0x7248,0x7348,0x7448,0x7548,0x7648,
0x7748,0x7848,0x7948,0x7a48,0x3048,0x3148,0x3248,0x3348,0x3448,0x3548,0x3648,0x3748,0x3848,0x3948,0x2b48,0x2f48,
0x4149,0x4249,0x4349,0x4449,0x4549,0x4649,0x4749,0x4849,0x4949,0x4a49,0x4b49,0x4c49,0x4d49,0x4e49,0x4f49,0x5049,
0x5149,0x5249,0x5349,0x5449,0x5549,0x5649,0x5749,0x5849,0x5949,0x5a49,0x6149,0x6249,0x6349,0x6449,0x6549,0x6649,
0x6749,0x6849,0x6949,0x6a49,0x6b49,0x6c49,0x6d49,0x6e49,0x6f49,0x7049,0x7149,0x7249,0x7349,0x7449,0x7549,0x7649,
0x7749,0x7849,0x7949,0x7a49,0x3049,0x3149,0x3249,0x3349,0x3449,0x3549,0x3649,0x3749,0x3849,0x3949,0x2b49,0x2f49,
0x414a,0x424a,0x434a,0x444a,0x454a,0x464a,0x474a,0x484a,0x494a,0x4a4a,0x4b4a,0x4c4a,0x4d4a,0x4e4a,0x4f4a,0x504a,
0x514a,0x524a,0x534a,0x544a,0x554a,0x564a,0x574a,0x584a,0x594a,0x5a4a,0x614a,0x624a,0x634a,0x644a,0x654a,0x664a,
0x674a,0x684a,0x694a,0x6a4a,0x6b4a,0x6c4a,0x6d4a,0x6e4a,0x6f4a,0x704a,0x714a,0x724a,0x734a,0x744a,0x754a,0x764a,
0x774a,0x784a,0x794a,0x7a4a,0x304a,0x314a,0x324a,0x334a,0x344a,0x354a,0x364a,0x374a,0x384a,0x394a,0x2b4a,0x2f4a,
0x414b,0x424b,0x434b,0x444b,0x454b,0x464b,0x474b,0x484b,0x494b,0x4a4b,0x4b4b,0x4c4b,0x4d4b,0x4e4b,0x4f4b,0x504b,
0x514b,0x524b,0x534b,0x544b,0x554b,0x564b,0x574b,0x584b,0x594b,0x5a4b,0x614b,0x624b,0x634b,0x644b,0x654b,0x664b,
0x674b,0x684b,0x694b,0x6a4b,0x6b4b,0x6c4b,0x6d4b,0x6e4b,0x6f4b,0x704b,0x714b,0x724b,0x734b,0x744b,0x754b,0x764b,
0x774b,0x784b,0x794b,0x7a4b,0x304b,0x314b,0x324b,0x334b,0x344b,0x354b,0x364b,0x374b,0x384b,0x394b,0x2b4b,0x2f4b,
0x414c,0x424c,0x434c,0x444c,0x454c,0x464c,0x474c,0x484c,0x494c,0x4a4c,0x4b4c,0x4c4c,0x4d4c,0x4e4c,0x4f4c,0x504c,
0x514c,0x524c,0x534c,0x544c,0x554c,0x564c,0x574c,0x584c,0x594c,0x5a4c,0x614c,0x624c,0x634c,0x644c,0x654c,0x664c,
0x674c,0x684c,0x694c,0x6a4c,0x6b4c,0x6c4c,0x6d4c,0x6e4c,0x6f4c,0x704c,0x714c,0x724c,0x734c,0x744c,0x754c,0x764c,
0x774c,0x784c,0x794c,0x7a4c,0x304c,0x314c,0x324c,0x334c,0x344c,0x354c,0x364c,0x374c,0x384c,0x394c,0x2b4c,0x2f4c,
0x414d,0x424d,0x434d,0x444d,0x454d,0x464d,0x474d,0x484d,0x494d,0x4a4d,0x4b4d,0x4c4d,0x4d4d,0x4e4d,0x4f4d,0x504d,
0x514d,0x524d,0x534d,0x544d,0x554d,0x564d,0x574d,0x584d,0x594d,0x5a4d,0x614d,0x624d,0x634d,0x644d,0x654d,0x664d,
0x674d,0x684d,0x694d,0x6a4d,0x6b4d,0x6c4d,0x6d4d,0x6e4d,0x6f4d,0x704d,0x714d,0x724d,0x734d,0x744d,0x754d,0x764d,
0x774d,0x784d,0x794d,0x7a4d,0x304d,0x314d,0x324d,0x334d,0x344d,0x354d,0x364d,0x374d,0x384d,0x394d,0x2b4d,0x2f4d,
0x414e,0x424e,0x434e,0x444e,0x454e,0x464e,0x474e,0x484e,0x494e,0x4a4e,0x4b4e,0x4c4e,0x4d4e,0x4e4e,0x4f4e,0x504e,
0x514e,0x524e,0x534e,0x544e,0x554e,0x564e,0x574e,0x584e,0x594e,0x5a4e,0x614e,0x624e,0x634e,0x644e,0x654e,0x664e,
0x674e,0x684e,0x694e,0x6a4e,0x6b4e,0x6c4e,0x6d4e,0x6e4e,0x6f4e,0x704e,0x714e,0x724e,0x734e,0x744e,0x754e,0x764e,
0x774e,0x784e,0x794e,0x7a4e,0x304e,0x314e,0x324e,0x334e,0x344e,0x354e,0x364e,0x374e,0x384e,0x394e,0x2b4e,0x2f4e,
0x414f,0x424f,0x434f,0x444f,0x454f,0x464f,0x474f,0x484f,0x494f,0x4a4f,0x4b4f,0x4c4f,0x4d4f,0x4e4f,0x4f4f,0x504f,
0x514f,0x524f,0x534f,0x544f,0x554f,0x564f,0x574f,0x584f,0x594f,0x5a4f,0x614f,0x624f,0x634f,0x644f,0x654f,0x664f,
0x674f,0x684f,0x694f,0x6a4f,0x6b4f,0x6c4f,0x6d4f,0x6e4f,0x6f4f,0x704f,0x714f,0x724f,0x734f,0x744f,0x754f,0x764f,
0x774f,0x784f,0x794f,0x7a4f,0x304f,0x314f,0x324f,0x334f,0x344f,0x354f,0x364f,0x374f,0x384f,0x394f,0x2b4f,0x2f4f,
0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,0x4a50,0x4b50,0x4c50,0x4d50,0x4e50,0x4f50,0x5050,
0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,0x5a50,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,
0x6750,0x6850,0x6950,0x6a50,0x6b50,0x6c50,0x6d50,0x6e50,0x6f50,0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,
0x7750,0x7850,0x7950,0x7a50,0x3050,0x3150,0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,0x2b50,0x2f50,
0x4151,0x4251,0x4351,0x4451,0x4551,0x4651,0x4751,0x4851,0x4951,0x4a51,0x4b51,0x4c51,0x4d51,0x4e51,0x4f51,0x5051,
0x5151,0x5251,0x5351,0x5451,0x5551,0x5651,0x5751,0x5851,0x5951,0x5a51,0x6151,0x6251,0x6351,0x6451,0x6551,0x6651,
0x6751,0x6851,0x6951,0x6a51,0x6b51,0x6c51,0x6d51,0x6e51,0x6f51,0x7051,0x7151,0x7251,0x7351,0x7451,0x7551,0x7651,
0x7751,0x7851,0x7951,0x7a51,0x3051,0x3151,0x3251,0x3351,0x3451,0x3551,0x3651,0x3751,0x3851,0x3951,0x2b51,0x2f51,
0x4152,0x4252,0x4352,0x4452,0x4552,0x4652,0x4752,0x4852,0x4952,0x4a52,0x4b52,0x4c52,0x4d52,0x4e52,0x4f52,0x5052,
0x5152,0x5252,0x5352,0x5452,0x5552,0x5652,0x5752,0x5852,0x5952,0x5a52,0x6152,0x6252,0x6352,0x6452,0x6552,0x6652,
0x6752,0x6852,0x6952,0x6a52,0x6b52,0x6c52,0x6d52,0x6e52,0x6f52,0x7052,0x7152,0x7252,0x7352,0x7452,0x7552,0x7652,
0x7752,0x7852,0x7952,0x7a52,0x3052,0x3152,0x3252,0x3352,0x3452,0x3552,0x3652,0x3752,0x3852,0x3952,0x2b52,0x2f52,
0x4153,0x4253,0x4353,0x4453,0x4553,0x4653,0x4753,0x4853,0x4953,0x4a53,0x4b53,0x4c53,0x4d53,0x4e53,0x4f53,0x5053,
0x5153,0x5253,0x5353,0x5453,0x5553,0x5653,0x5753,0x5853,0x5953,0x5a53,0x6153,0x6253,0x6353,0x6453,0x6553,0x6653,
0x6753,0x6853,0x6953,0x6a53,0x6b53,0x6c53,0x6d53,0x6e53,0x6f53,0x7053,0x7153,0x7253,0x7353,0x7453,0x7553,0x7653,
0x7753,0x7853,0x7953,0x7a53,0x3053,0x3153,0x3253,0x3353,0x3453,0x3553,0x3653,0x3753,0x3853,0x3953,0x2b53,0x2f53,
0x4154,0x4254,0x4354,0x4454,0x4554,0x4654,0x4754,0x4854,0x4954,0x4a54,0x4b54,0x4c54,0x4d54,0x4e54,0x4f54,0x5054,
0x5154,0x5254,0x5354,0x5454,0x5554,0x5654,0x5754,0x5854,0x5954,0x5a54,0x6154,0x6254,0x6354,0x6454,0x6554,0x6654,
0x6754,0x6854,0x6954,0x6a54,0x6b54,0x6c54,0x6d54,0x6e54,0x6f54,0x7054,0x7154,0x7254,0x7354,0x7454,0x7554,0x7654,
0x7754,0x7854,0x7954,0x7a54,0x3054,0x3154,0x3254,0x3354,0x3454,0x3554,0x3654,0x3754,0x3854,0x3954,0x2b54,0x2f54,
0x4155,0x4255,0x4355,0x4455,0x4555,0x4655,0x4755,0x4855,0x4955,0x4a55,0x4b55,0x4c55,0x4d55,0x4e55,0x4f55,0x5055,
0x5155,0x5255,0x5355,0x5455,0x5555,0x5655,0x5755,0x5855,0x5955,0x5a55,0x6155,0x6255,0x6355,0x6455,0x6555,0x6655,
0x6755,0x6855,0x6955,0x6a55,0x6b55,0x6c55,0x6d55,0x6e55,0x6f55,0x7055,0x7155,0x7255,0x7355,0x7455,0x7555,0x7655,
0x7755,0x7855,0x7955,0x7a55,0x3055,0x3155,0x3255,0x3355,0x3455,0x3555,0x3655,0x3755,0x3855,0x3955,0x2b55,0x2f55,
0x4156,0x4256,0x4356,0x4456,0x4556,0x4656,0x4756,0x4856,0x4956,0x4a56,0x4b56,0x4c56,0x4d56,0x4e56,0x4f56,0x5056,
0x5156,0x5256,0x5356,0x5456,0x5556,0x5656,0x5756,0x5856,0x5956,0x5a56,0x6156,0x6256,0x6356,0x6456,0x6556,0x6656,
0x6756,0x6856,0x6956,0x6a56,0x6b56,0x6c56,0x6d56,0x6e56,0x6f56,0x7056,0x7156,0x7256,0x7356,0x7456,0x7556,0x7656,
0x7756,0x7856,0x7956,0x7a56,0x3056,0x3156,0x3256,0x3356,0x3456,0x3556,0x3656,0x3756,0x3856,0x3956,0x2b56,0x2f56,
0x4157,0x4257,0x4357,0x4457,0x4557,0x4657,0x4757,0x4857,0x4957,0x4a57,0x4b57,0x4c57,0x4d57,0x4e57,0x4f57,0x5057,
0x5157,0x5257,0x5357,0x5457,0x5557,0x5657,0x5757,0x5857,0x5957,0x5a57,0x6157,0x6257,0x6357,0x6457,0x6557,0x6657,
0x6757,0x6857,0x6957,0x6a57,0x6b57,0x6c57,0x6d57,0x6e57,0x6f57,0x7057,0x7157,0x7257,0x7357,0x7457,0x7557,0x7657,
0x7757,0x7857,0x7957,0x7a57,0x3057,0x3157,0x3257,0x3357,0x3457,0x3557,0x3657,0x3757,0x3857,0x3957,0x2b57,0x2f57,
0x4158,0x4258,0x4358,0x4458,0x4558,0x4658,0x4758,0x4858,0x4958,0x4a58,0x4b58,0x4c58,0x4d58,0x4e58,0x4f58,0x5058,
0x5158,0x5258,0x5358,0x5458,0x5558,0x5658,0x5758,0x5858,0x5958,0x5a58,0x6158,0x6258,0x6358,0x6458,0x6558,0x6658,
0x6758,0x6858,0x6958,0x6a58,0x6b58,0x6c58,0x6d58,0x6e58,0x6f58,0x7058,0x7158,0x7258,0x7358,0x7458,0x7558,0x7658,
0x7758,0x7858,0x7958,0x7a58,0x3058,0x3158,0x3258,0x3358,0x3458,0x3558,0x3658,0x3758,0x3858,0x3958,0x2b58,0x2f58,
0x4159,0x4259,0x4359,0x4459,0x4559,0x4659,0x4759,0x4859,0x4959,0x4a59,0x4b59,0x4c59,0x4d59,0x4e59,0x4f59,0x5059,
0x5159,0x5259,0x5359,0x5459,0x5559,0x5659,0x5759,0x5859,0x5959,0x5a59,0x6159,0x6259,0x6359,0x6459,0x6559,0x6659,
0x6759,0x6859,0x6959,0x6a59,0x6b59,0x6c59,0x6d59,0x6e59,0x6f59,0x7059,0x7159,0x7259,0x7359,0x7459,0x7559,0x7659,
0x7759,0x7859,0x7959,0x7a59,0x3059,0x3159,0x3259,0x3359,0x3459,0x3559,0x3659,0x3759,0x3859,0x3959,0x2b59,0x2f59,
0x415a,0x425a,0x435a,0x445a,0x455a,0x465a,0x475a,0x485a,0x495a,0x4a5a,0x4b5a,0x4c5a,0x4d5a,0x4e5a,0x4f5a,0x505a,
0x515a,0x525a,0x535a,0x545a,0x555a,0x565a,0x575a,0x585a,0x595a,0x5a5a,0x615a,0x625a,0x635a,0x645a,0x655a,0x665a,
0x675a,0x685a,0x695a,0x6a5a,0x6b5a,0x6c5a,0x6d5a,0x6e5a,0x6f5a,0x705a,0x715a,0x725a,0x735a,0x745a,0x755a,0x765a,
0x775a,0x785a,0x795a,0x7a5a,0x305a,0x315a,0x325a,0x335a,0x345a,0x355a,0x365a,0x375a,0x385a,0x395a,0x2b5a,0x2f5a,
0x4161,0x4261,0x4361,0x4461,0x4561,0x4661,0x4761,0x4861,0x4961,0x4a61,0x4b61,0x4c61,0x4d61,0x4e61,0x4f61,0x5061,
0x5161,0x5261,0x5361,0x5461,0x5561,0x5661,0x5761,0x5861,0x5961,0x5a61,0x6161,0x6261,0x6361,0x6461,0x6561,0x6661,
0x6761,0x6861,0x6961,0x6a61,0x6b61,0x6c61,0x6d61,0x6e61,0x6f61,0x7061,0x7161,0x7261,0x7361,0x7461,0x7561,0x7661,
0x7761,0x7861,0x7961,0x7a61,0x3061,0x3161,0x3261,0x3361,0x3461,0x3561,0x3661,0x3761,0x3861,0x3961,0x2b61,0x2f61,
0x4162,0x4262,0x4362,0x4462,0x4562,0x4662,0x4762,0x4862,0x4962,0x4a62,0x4b62,0x4c62,0x4d62,0x4e62,0x4f62,0x5062,
0x5162,0x5262,0x5362,0x5462,0x5562,0x5662,0x5762,0x5862,0x5962,0x5a62,0x6162,0x6262,0x6362,0x6462,0x6562,0x6662,
0x6762,0x6862,0x6962,0x6a62,0x6b62,0x6c62,0x6d62,0x6e62,0x6f62,0x7062,0x7162,0x7262,0x7362,0x7462,0x7562,0x7662,
0x7762,0x7862,0x7962,0x7a62,0x3062,0x3162,0x3262,0x3362,0x3462,0x3562,0x3662,0x3762,0x3862,0x3962,0x2b62,0x2f62,
0x4163,0x4263,0x4363,0x4463,0x4563,0x4663,0x4763,0x4863,0x4963,0x4a63,0x4b63,0x4c63,0x4d63,0x4e63,0x4f63,0x5063,
0x5163,0x5263,0x5363,0x5463,0x5563,0x5663,0x5763,0x5863,0x5963,0x5a63,0x6163,0x6263,0x6363,0x6463,0x6563,0x6663,
0x6763,0x6863,0x6963,0x6a63,0x6b63,0x6c63,0x6d63,0x6e63,0x6f63,0x7063,0x7163,0x7263,0x7363,0x7463,0x7563,0x7663,
0x7763,0x7863,0x7963,0x7a63,0x3063,0x3163,0x3263,0x3363,0x3463,0x3563,0x3663,0x3763,0x3863,0x3963,0x2b63,0x2f63,
0x4164,0x4264,0x4364,0x4464,0x4564,0x4664,0x4764,0x4864,0x4964,0x4a64,0x4b64,0x4c64,0x4d64,0x4e64,0x4f64,0x5064,
0x5164,0x5264,0x5364,0x5464,0x5564,0x5664,0x5764,0x5864,0x5964,0x5a64,0x6164,0x6264,0x6364,0x6464,0x6564,0x6664,
0x6764,0x6864,0x6964,0x6a64,0x6b64,0x6c64,0x6d64,0x6e64,0x6f64,0x7064,0x7164,0x7264,0x7364,0x7464,0x7564,0x7664,
0x7764,0x7864,0x7964,0x7a64,0x3064,0x3164,0x3264,0x3364,0x3464,0x3564,0x3664,0x3764,0x3864,0x3964,0x2b64,0x2f64,
0x4165,0x4265,0x4365,0x4465,0x4565,0x4665,0x4765,0x4865,0x4965,0x4a65,0x4b65,0x4c65,0x4d65,0x4e65,0x4f65,0x5065,
0x5165,0x5265,0x5365,0x5465,0x5565,0x5665,0x5765,0x5865,0x5965,0x5a65,0x6165,0x6265,0x6365,0x6465,0x6565,0x6665,
0x6765,0x6865,0x6965,0x6a65,0x6b65,0x6c65,0x6d65,0x6e65,0x6f65,0x7065,0x7165,0x7265,0x7365,0x7465,0x7565,0x7665,
0x7765,0x7865,0x7965,0x7a65,0x3065,0x3165,0x3265,0x3365,0x3465,0x3565,0x3665,0x3765,0x3865,0x3965,0x2b65,0x2f65,
0x4166,0x4266,0x4366,0x4466,0x4566,0x4666,0x4766,0x4866,0x4966,0x4a66,0x4b66,0x4c66,0x4d66,0x4e66,0x4f66,0x5066,
0x5166,0x5266,0x5366,0x5466,0x5566,0x5666,0x5766,0x5866,0x5966,0x5a66,0x6166,0x6266,0x6366,0x6466,0x6566,0x6666,
0x6766,0x6866,0x6966,0x6a66,0x6b66,0x6c66,0x6d66,0x6e66,0x6f66,0x7066,0x7166,0x7266,0x7366,0x7466,0x7566,0x7666,
0x7766,0x7866,0x7966,0x7a66,0x3066,0x3166,0x3266,0x3366,0x3466,0x3566,0x3666,0x3766,0x3866,0x3966,0x2b66,0x2f66,
0x4167,0x4267,0x4367,0x4467,0x4567,0x4667,0x4767,0x4867,0x4967,0x4a67,0x4b67,0x4c67,0x4d67,0x4e67,0x4f67,0x5067,
0x5167,0x5267,0x5367,0x5467,0x5567,0x5667,0x5767,0x5867,0x5967,0x5a67,0x6167,0x6267,0x6367,0x6467,0x6567,0x6667,
0x6767,0x6867,0x6967,0x6a67,0x6b67,0x6c67,0x6d67,0x6e67,0x6f67,0x7067,0x7167,0x7267,0x7367,0x7467,0x7567,0x7667,
0x7767,0x7867,0x7967,0x7a67,0x3067,0x3167,0x3267,0x3367,0x3467,0x3567,0x3667,0x3767,0x3867,0x3967,0x2b67,0x2f67,
0x4168,0x4268,0x4368,0x4468,0x4568,0x4668,0x4768,0x4868,0x4968,0x4a68,0x4b68,0x4c68,0x4d68,0x4e68,0x4f68,0x5068,
0x5168,0x5268,0x5368,0x5468,0x5568,0x5668,0x5768,0x5868,0x5968,0x5a68,0x6168,0x6268,0x6368,0x6468,0x6568,0x6668,
0x6768,0x6868,0x6968,0x6a68,0x6b68,0x6c68,0x6d68,0x6e68,0x6f68,0x7068,0x7168,0x7268,0x7368,0x7468,0x7568,0x7668,
0x7768,0x7868,0x7968,0x7a68,0x3068,0x3168,0x3268,0x3368,0x3468,0x3568,0x3668,0x3768,0x3868,0x3968,0x2b68,0x2f68,
0x4169,0x4269,0x4369,0x4469,0x4569,0x4669,0x4769,0x4869,0x4969,0x4a69,0x4b69,0x4c69,0x4d69,0x4e69,0x4f69,0x5069,
0x5169,0x5269,0x5369,0x5469,0x5569,0x5669,0x5769,0x5869,0x5969,0x5a69,0x6169,0x6269,0x6369,0x6469,0x6569,0x6669,
0x6769,0x6869,0x6969,0x6a69,0x6b69,0x6c69,0x6d69,0x6e69,0x6f69,0x7069,0x7169,0x7269,0x7369,0x7469,0x7569,0x7669,
0x7769,0x7869,0x7969,0x7a69,0x3069,0x3169,0x3269,0x3369,0x3469,0x3569,0x3669,0x3769,0x3869,0x3969,0x2b69,0x2f69,
0x416a,0x426a,0x436a,0x446a,0x456a,0x466a,0x476a,0x486a,0x496a,0x4a6a,0x4b6a,0x4c6a,0x4d6a,0x4e6a,0x4f6a,0x506a,
0x516a,0x526a,0x536a,0x546a,0x556a,0x566a,0x576a,0x586a,0x596a,0x5a6a,0x616a,0x626a,0x636a,0x646a,0x656a,0x666a,
0x676a,0x686a,0x696a,0x6a6a,0x6b6a,0x6c6a,0x6d6a,0x6e6a,0x6f6a,0x706a,0x716a,0x726a,0x736a,0x746a,0x756a,0x766a,
0x776a,0x786a,0x796a,0x7a6a,0x306a,0x316a,0x326a,0x336a,0x346a,0x356a,0x366a,0x376a,0x386a,0x396a,0x2b6a,0x2f6a,
0x416b,0x426b,0x436b,0x446b,0x456b,0x466b,0x476b,0x486b,0x496b,0x4a6b,0x4b6b,0x4c6b,0x4d6b,0x4e6b,0x4f6b,0x506b,
0x516b,0x526b,0x536b,0x546b,0x556b,0x566b,0x576b,0x586b,0x596b,0x5a6b,0x616b,0x626b,0x636b,0x646b,0x656b,0x666b,
0x676b,0x686b,0x696b,0x6a6b,0x6b6b,0x6c6b,0x6d6b,0x6e6b,0x6f6b,0x706b,0x716b,0x726b,0x736b,0x746b,0x756b,0x766b,
0x776b,0x786b,0x796b,0x7a6b,0x306b,0x316b,0x326b,0x336b,0x346b,0x356b,0x366b,0x376b,0x386b,0x396b,0x2b6b,0x2f6b,
0x416c,0x426c,0x436c,0x446c,0x456c,0x466c,0x476c,0x486c,0x496c,0x4a6c,0x4b6c,0x4c6c,0x4d6c,0x4e6c,0x4f6c,0x506c,
0x516c,0x526c,0x536c,0x546c,0x556c,0x566c,0x576c,0x586c,0x596c,0x5a6c,0x616c,0x626c,0x636c,0x646c,0x656c,0x666c,
0x676c,0x686c,0x696c,0x6a6c,0x6b6c,0x6c6c,0x6d6c,0x6e6c,0x6f6c,0x706c,0x716c,0x726c,0x736c,0x746c,0x756c,0x766c,
0x776c,0x786c,0x796c,0x7a6c,0x306c,0x316c,0x326c,0x336c,0x346c,0x356c,0x366c,0x376c,0x386c,0x396c,0x2b6c,0x2f6c,
0x416d,0x426d,0x436d,0x446d,0x456d,0x466d,0x476d,0x486d,0x496d,0x4a6d,0x4b6d,0x4c6d,0x4d6d,0x4e6d,0x4f6d,0x506d,
0x516d,0x526d,0x536d,0x546d,0x556d,0x566d,0x576d,0x586d,0x596d,0x5a6d,0x616d,0x626d,0x636d,0x646d,0x656d,0x666d,
0x676d,0x686d,0x696d,0x6a6d,0x6b6d,0x6c6d,0x6d6d,0x6e6d,0x6f6d,0x706d,0x716d,0x726d,0x736d,0x746d,0x756d,0x766d,
0x776d,0x786d,0x796d,0x7a6d,0x306d,0x316d,0x326d,0x336d,0x346d,0x356d,0x366d,0x376d,0x386d,0x396d,0x2b6d,0x2f6d,
0x416e,0x426e,0x436e,0x446e,0x456e,0x466e,0x476e,0x486e,0x496e,0x4a6e,0x4b6e,0x4c6e,0x4d6e,0x4e6e,0x4f6e,0x506e,
0x516e,0x526e,0x536e,0x546e,0x556e,0x566e,0x576e,0x586e,0x596e,0x5a6e,0x616e,0x626e,0x636e,0x646e,0x656e,0x666e,
0x676e,0x686e,0x696e,0x6a6e,0x6b6e,0x6c6e,0x6d6e,0x6e6e,0x6f6e,0x706e,0x716e,0x726e,0x736e,0x746e,0x756e,0x766e,
0x776e,0x786e,0x796e,0x7a6e,0x306e,0x316e,0x326e,0x336e,0x346e,0x356e,0x366e,0x376e,0x386e,0x396e,0x2b6e,0x2f6e,
0x416f,0x426f,0x436f,0x446f,0x456f,0x466f,0x476f,0x486f,0x496f,0x4a6f,0x4b6f,0x4c6f,0x4d6f,0x4e6f,0x4f6f,0x506f,
0x516f,0x526f,0x536f,0x546f,0x556f,0x566f,0x576f,0x586f,0x596f,0x5a6f,0x616f,0x626f,0x636f,0x646f,0x656f,0x666f,
0x676f,0x686f,0x696f,0x6a6f,0x6b6f,0x6c6f,0x6d6f,0x6e6f,0x6f6f,0x706f,0x716f,0x726f,0x736f,0x746f,0x756f,0x766f,
0x776f,0x786f,0x796f,0x7a6f,0x306f,0x316f,0x326f,0x336f,0x346f,0x356f,0x366f,0x376f,0x386f,0x396f,0x2b6f,0x2f6f,
0x4170,0x4270,0x4370,0x4470,0x4570,0x4670,0x4770,0x4870,0x4970,0x4a70,0x4b70,0x4c70,0x4d70,0x4e70,0x4f70,0x5070,
0x5170,0x5270,0x5370,0x5470,0x5570,0x5670,0x5770,0x5870,0x5970,0x5a70,0x6170,0x6270,0x6370,0x6470,0x6570,0x6670,
0x6770,0x6870,0x6970,0x6a70,0x6b70,0x6c70,0x6d70,0x6e70,0x6f70,0x7070,0x7170,0x7270,0x7370,0x7470,0x7570,0x7670,
0x7770,0x7870,0x7970,0x7a70,0x3070,0x3170,0x3270,0x3370,0x3470,0x3570,0x3670,0x3770,0x3870,0x3970,0x2b70,0x2f70,
0x4171,0x4271,0x4371,0x4471,0x4571,0x4671,0x4771,0x4871,0x4971,0x4a71,0x4b71,0x4c71,0x4d71,0x4e71,0x4f71,0x5071,
0x5171,0x5271,0x5371,0x5471,0x5571,0x5671,0x5771,0x5871,0x5971,0x5a71,0x6171,0x6271,0x6371,0x6471,0x6571,0x6671,
0x6771,0x6871,0x6971,0x6a71,0x6b71,0x6c71,0x6d71,0x6e71,0x6f71,0x7071,0x7171,0x7271,0x7371,0x7471,0x7571,0x7671,
0x7771,0x7871,0x7971,0x7a71,0x3071,0x3171,0x3271,0x3371,0x3471,0x3571,0x3671,0x3771,0x3871,0x3971,0x2b71,0x2f71,
0x4172,0x4272,0x4372,0x4472,0x4572,0x4672,0x4772,0x4872,0x4972,0x4a72,0x4b72,0x4c72,0x4d72,0x4e72,0x4f72,0x5072,
0x5172,0x5272,0x5372,0x5472,0x5572,0x5672,0x5772,0x5872,0x5972,0x5a72,0x6172,0x6272,0x6372,0x6472,0x6572,0x6672,
0x6772,0x6872,0x6972,0x6a72,0x6b72,0x6c72,0x6d72,0x6e72,0x6f72,0x7072,0x7172,0x7272,0x7372,0x7472,0x7572,0x7672,
0x7772,0x7872,0x7972,0x7a72,0x3072,0x3172,0x3272,0x3372,0x3472,0x3572,0x3672,0x3772,0x3872,0x3972,0x2b72,0x2f72,
0x4173,0x4273,0x4373,0x4473,0x4573,0x4673,0x4773,0x4873,0x4973,0x4a73,0x4b73,0x4c73,0x4d73,0x4e73,0x4f73,0x5073,
0x5173,0x5273,0x5373,0x5473,0x5573,0x5673,0x5773,0x5873,0x5973,0x5a73,0x6173,0x6273,0x6373,0x6473,0x6573,0x6673,
0x6773,0x6873,0x6973,0x6a73,0x6b73,0x6c73,0x6d73,0x6e73,0x6f73,0x7073,0x7173,0x7273,0x7373,0x7473,0x7573,0x7673,
0x7773,0x7873,0x7973,0x7a73,0x3073,0x3173,0x3273,0x3373,0x3473,0x3573,0x3673,0x3773,0x3873,0x3973,0x2b73,0x2f73,
0x4174,0x4274,0x4374,0x4474,0x4574,0x4674,0x4774,0x4874,0x4974,0x4a74,0x4b74,0x4c74,0x4d74,0x4e74,0x4f74,0x5074,
0x5174,0x5274,0x5374,0x5474,0x5574,0x5674,0x5774,0x5874,0x5974,0x5a74,0x6174,0x6274,0x6374,0x6474,0x6574,0x6674,
0x6774,0x6874,0x6974,0x6a74,0x6b74,0x6c74,0x6d74,0x6e74,0x6f74,0x7074,0x7174,0x7274,0x7374,0x7474,0x7574,0x7674,
0x7774,0x7874,0x7974,0x7a74,0x3074,0x3174,0x3274,0x3374,0x3474,0x3574,0x3674,0x3774,0x3874,0x3974,0x2b74,0x2f74,
0x4175,0x4275,0x4375,0x4475,0x4575,0x4675,0x4775,0x4875,0x4975,0x4a75,0x4b75,0x4c75,0x4d75,0x4e75,0x4f75,0x5075,
0x5175,0x5275,0x5375,0x5475,0x5575,0x5675,0x5775,0x5875,0x5975,0x5a75,0x6175,0x6275,0x6375,0x6475,0x6575,0x6675,
0x6775,0x6875,0x6975,0x6a75,0x6b75,0x6c75,0x6d75,0x6e75,0x6f75,0x7075,0x7175,0x7275,0x7375,0x7475,0x7575,0x7675,
0x7775,0x7875,0x7975,0x7a75,0x3075,0x3175,0x3275,0x3375,0x3475,0x3575,0x3675,0x3775,0x3875,0x3975,0x2b75,0x2f75,
0x4176,0x4276,0x4376,0x4476,0x4576,0x4676,0x4776,0x4876,0x4976,0x4a76,0x4b76,0x4c76,0x4d76,0x4e76,0x4f76,0x5076,
0x5176,0x5276,0x5376,0x5476,0x5576,0x5676,0x5776,0x5876,0x5976,0x5a76,0x6176,0x6276,0x6376,0x6476,0x6576,0x6676,
0x6776,0x6876,0x6976,0x6a76,0x6b76,0x6c76,0x6d76,0x6e76,0x6f76,0x7076,0x7176,0x7276,0x7376,0x7476,0x7576,0x7676,
0x7776,0x7876,0x7976,0x7a76,0x3076,0x3176,0x3276,0x3376,0x3476,0x3576,0x3676,0x3776,0x3876,0x3976,0x2b76,0x2f76,
0x4177,0x4277,0x4377,0x4477,0x4577,0x4677,0x4777,0x4877,0x4977,0x4a77,0x4b77,0x4c77,0x4d77,0x4e77,0x4f77,0x5077,
0x5177,0x5277,0x5377,0x5477,0x5577,0x5677,0x5777,0x5877,0x5977,0x5a77,0x6177,0x6277,0x6377,0x6477,0x6577,0x6677,
0x6777,0x6877,0x6977,0x6a77,0x6b77,0x6c77,0x6d77,0x6e77,0x6f77,0x7077,0x7177,0x7277,0x7377,0x7477,0x7577,0x7677,
0x7777,0x7877,0x7977,0x7a77,0x3077,0x3177,0x3277,0x3377,0x3477,0x3577,0x3677,0x3777,0x3877,0x3977,0x2b77,0x2f77,
0x4178,0x4278,0x4378,0x4478,0x4578,0x4678,0x4778,0x4878,0x4978,0x4a78,0x4b78,0x4c78,0x4d78,0x4e78,0x4f78,0x5078,
0x5178,0x5278,0x5378,0x5478,0x5578,0x5678,0x5778,0x5878,0x5978,0x5a78,0x6178,0x6278,0x6378,0x6478,0x6578,0x6678,
0x6778,0x6878,0x6978,0x6a78,0x6b78,0x6c78,0x6d78,0x6e78,0x6f78,0x7078,0x7178,0x7278,0x7378,0x7478,0x7578,0x7678,
0x7778,0x7878,0x7978,0x7a78,0x3078,0x3178,0x3278,0x3378,0x3478,0x3578,0x3678,0x3778,0x3878,0x3978,0x2b78,0x2f78,
0x4179,0x4279,0x4379,0x4479,0x4579,0x4679,0x4779,0x4879,0x4979,0x4a79,0x4b79,0x4c79,0x4d79,0x4e79,0x4f79,0x5079,
0x5179,0x5279,0x5379,0x5479,0x5579,0x5679,0x5779,0x5879,0x5979,0x5a79,0x6179,0x6279,0x6379,0x6479,0x6579,0x6679,
0x6779,0x6879,0x6979,0x6a79,0x6b79,0x6c79,0x6d79,0x6e79,0x6f79,0x7079,0x7179,0x7279,0x7379,0x7479,0x7579,0x7679,
0x7779,0x7879,0x7979,0x7a79,0x3079,0x3179,0x3279,0x3379,0x3479,0x3579,0x3679,0x3779,0x3879,0x3979,0x2b79,0x2f79,
0x417a,0x427a,0x437a,0x447a,0x457a,0x467a,0x477a,0x487a,0x497a,0x4a7a,0x4b7a,0x4c7a,0x4d7a,0x4e7a,0x4f7a,0x507a,
0x517a,0x527a,0x537a,0x547a,0x557a,0x567a,0x577a,0x587a,0x597a,0x5a7a,0x617a,0x627a,0x637a,0x647a,0x657a,0x667a,
0x677a,0x687a,0x697a,0x6a7a,0x6b7a,0x6c7a,0x6d7a,0x6e7a,0x6f7a,0x707a,0x717a,0x727a,0x737a,0x747a,0x757a,0x767a,
0x777a,0x787a,0x797a,0x7a7a,0x307a,0x317a,0x327a,0x337a,0x347a,0x357a,0x367a,0x377a,0x387a,0x397a,0x2b7a,0x2f7a,
0x4130,0x4230,0x4330,0x4430,0x4530,0x4630,0x4730,0x4830,0x4930,0x4a30,0x4b30,0x4c30,0x4d30,0x4e30,0x4f30,0x5030,
0x5130,0x5230,0x5330,0x5430,0x5530,0x5630,0x5730,0x5830,0x5930,0x5a30,0x6130,0x6230,0x6330,0x6430,0x6530,0x6630,
0x6730,0x6830,0x6930,0x6a30,0x6b30,0x6c30,0x6d30,0x6e30,0x6f30,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,0x7630,
0x7730,0x7830,0x7930,0x7a30,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,0x3630,0x3730,0x3830,0x3930,0x2b30,0x2f30,
0x4131,0x4231,0x4331,0x4431,0x4531,0x4631,0x4731,0x4831,0x4931,0x4a31,0x4b31,0x4c31,0x4d31,0x4e31,0x4f31,0x5031,
0x5131,0x5231,0x5331,0x5431,0x5531,0x5631,0x5731,0x5831,0x5931,0x5a31,0x6131,0x6231,0x6331,0x6431,0x6531,0x6631,
0x6731,0x6831,0x6931,0x6a31,0x6b31,0x6c31,0x6d31,0x6e31,0x6f31,0x7031,0x7131,0x7231,0x7331,0x7431,0x7531,0x7631,
0x7731,0x7831,0x7931,0x7a31,0x3031,0x3131,0x3231,0x3331,0x3431,0x3531,0x3631,0x3731,0x3831,0x3931,0x2b31,0x2f31,
0x4132,0x4232,0x4332,0x4432,0x4532,0x4632,0x4732,0x4832,0x4932,0x4a32,0x4b32,0x4c32,0x4d32,0x4e32,0x4f32,0x5032,
0x5132,0x5232,0x5332,0x5432,0x5532,0x5632,0x5732,0x5832,0x5932,0x5a32,0x6132,0x6232,0x6332,0x6432,0x6532,0x6632,
0x6732,0x6832,0x6932,0x6a32,0x6b32,0x6c32,0x6d32,0x6e32,0x6f32,0x7032,0x7132,0x7232,0x7332,0x7432,0x7532,0x7632,
0x7732,0x7832,0x7932,0x7a32,0x3032,0x3132,0x3232,0x3332,0x3432,0x3532,0x3632,0x3732,0x3832,0x3932,0x2b32,0x2f32,
0x4133,0x4233,0x4333,0x4433,0x4533,0x4633,0x4733,0x4833,0x4933,0x4a33,0x4b33,0x4c33,0x4d33,0x4e33,0x4f33,0x5033,
0x5133,0x5233,0x5333,0x5433,0x5533,0x5633,0x5733,0x5833,0x5933,0x5a33,0x6133,0x6233,0x6333,0x6433,0x6533,0x6633,
0x6733,0x6833,0x6933,0x6a33,0x6b33,0x6c33,0x6d33,0x6e33,0x6f33,0x7033,0x7133,0x7233,0x7333,0x7433,0x7533,0x7633,
0x7733,0x7833,0x7933,0x7a33,0x3033,0x3133,0x3233,0x3333,0x3433,0x3533,0x3633,0x3733,0x3833,0x3933,0x2b33,0x2f33,
0x4134,0x4234,0x4334,0x4434,0x4534,0x4634,0x4734,0x4834,0x4934,0x4a34,0x4b34,0x4c34,0x4d34,0x4e34,0x4f34,0x5034,
0x5134,0x5234,0x5334,0x5434,0x5534,0x5634,0x5734,0x5834,0x5934,0x5a34,0x6134,0x6234,0x6334,0x6434,0x6534,0x6634,
0x6734,0x6834,0x6934,0x6a34,0x6b34,0x6c34,0x6d34,0x6e34,0x6f34,0x7034,0x7134,0x7234,0x7334,0x7434,0x7534,0x7634,
0x7734,0x7834,0x7934,0x7a34,0x3034,0x3134,0x3234,0x3334,0x3434,0x3534,0x3634,0x3734,0x3834,0x3934,0x2b34,0x2f34,
0x4135,0x4235,0x4335,0x4435,0x4535,0x4635,0x4735,0x4835,0x4935,0x4a35,0x4b35,0x4c35,0x4d35,0x4e35,0x4f35,0x5035,
0x5135,0x5235,0x5335,0x5435,0x5535,0x5635,0x5735,0x5835,0x5935,0x5a35,0x6135,0x6235,0x6335,0x6435,0x6535,0x6635,
0x6735,0x6835,0x6935,0x6a35,0x6b35,0x6c35,0x6d35,0x6e35,0x6f35,0x7035,0x7135,0x7235,0x7335,0x7435,0x7535,0x7635,
0x7735,0x7835,0x7935,0x7a35,0x3035,0x3135,0x3235,0x3335,0x3435,0x3535,0x3635,0x3735,0x3835,0x3935,0x2b35,0x2f35,
0x4136,0x4236,0x4336,0x4436,0x4536,0x4636,0x4736,0x4836,0x4936,0x4a36,0x4b36,0x4c36,0x4d36,0x4e36,0x4f36,0x5036,
0x5136,0x5236,0x5336,0x5436,0x5536,0x5636,0x5736,0x5836,0x5936,0x5a36,0x6136,0x6236,0x6336,0x6436,0x6536,0x6636,
0x6736,0x6836,0x6936,0x6a36,0x6b36,0x6c36,0x6d36,0x6e36,0x6f36,0x7036,0x7136,0x7236,0x7336,0x7436,0x7536,0x7636,
0x7736,0x7836,0x7936,0x7a36,0x3036,0x3136,0x3236,0x3336,0x3436,0x3536,0x3636,0x3736,0x3836,0x3936,0x2b36,0x2f36,
0x4137,0x4237,0x4337,0x4437,0x4537,0x4637,0x4737,0x4837,0x4937,0x4a37,0x4b37,0x4c37,0x4d37,0x4e37,0x4f37,0x5037,
0x5137,0x5237,0x5337,0x5437,0x5537,0x5637,0x5737,0x5837,0x5937,0x5a37,0x6137,0x6237,0x6337,0x6437,0x6537,0x6637,
0x6737,0x6837,0x6937,0x6a37,0x6b37,0x6c37,0x6d37,0x6e37,0x6f37,0x7037,0x7137,0x7237,0x7337,0x7437,0x7537,0x7637,
0x7737,0x7837,0x7937,0x7a37,0x3037,0x3137,0x3237,0x3337,0x3437,0x3537,0x3637,0x3737,0x3837,0x3937,0x2b37,0x2f37,
0x4138,0x4238,0x4338,0x4438,0x4538,0x4638,0x4738,0x4838,0x4938,0x4a38,0x4b38,0x4c38,0x4d38,0x4e38,0x4f38,0x5038,
0x5138,0x5238,0x5338,0x5438,0x5538,0x5638,0x5738,0x5838,0x5938,0x5a38,0x6138,0x6238,0x6338,0x6438,0x6538,0x6638,
0x6738,0x6838,0x6938,0x6a38,0x6b38,0x6c38,0x6d38,0x6e38,0x6f38,0x7038,0x7138,0x7238,0x7338,0x7438,0x7538,0x7638,
0x7738,0x7838,0x7938,0x7a38,0x3038,0x3138,0x3238,0x3338,0x3438,0x3538,0x3638,0x3738,0x3838,0x3938,0x2b38,0x2f38,
0x4139,0x4239,0x4339,0x4439,0x4539,0x4639,0x4739,0x4839,0x4939,0x4a39,0x4b39,0x4c39,0x4d39,0x4e39,0x4f39,0x5039,
0x5139,0x5239,0x5339,0x5439,0x5539,0x5639,0x5739,0x5839,0x5939,0x5a39,0x6139,0x6239,0x6339,0x6439,0x6539,0x6639,
0x6739,0x6839,0x6939,0x6a39,0x6b39,0x6c39,0x6d39,0x6e39,0x6f39,0x7039,0x7139,0x7239,0x7339,0x7439,0x7539,0x7639,
0x7739,0x7839,0x7939,0x7a39,0x3039,0x3139,0x3239,0x3339,0x3439,0x3539,0x3639,0x3739,0x3839,0x3939,0x2b39,0x2f39,
0x412b,0x422b,0x432b,0x442b,0x452b,0x462b,0x472b,0x482b,0x492b,0x4a2b,0x4b2b,0x4c2b,0x4d2b,0x4e2b,0x4f2b,0x502b,
0x512b,0x522b,0x532b,0x542b,0x552b,0x562b,0x572b,0x582b,0x592b,0x5a2b,0x612b,0x622b,0x632b,0x642b,0x652b,0x662b,
0x672b,0x682b,0x692b,0x6a2b,0x6b2b,0x6c2b,0x6d2b,0x6e2b,0x6f2b,0x702b,0x712b,0x722b,0x732b,0x742b,0x752b,0x762b,
0x772b,0x782b,0x792b,0x7a2b,0x302b,0x312b,0x322b,0x332b,0x342b,0x352b,0x362b,0x372b,0x382b,0x392b,0x2b2b,0x2f2b,
0x412f,0x422f,0x432f,0x442f,0x452f,0x462f,0x472f,0x482f,0x492f,0x4a2f,0x4b2f,0x4c2f,0x4d2f,0x4e2f,0x4f2f,0x502f,
0x512f,0x522f,0x532f,0x542f,0x552f,0x562f,0x572f,0x582f,0x592f,0x5a2f,0x612f,0x622f,0x632f,0x642f,0x652f,0x662f,
0x672f,0x682f,0x692f,0x6a2f,0x6b2f,0x6c2f,0x6d2f,0x6e2f,0x6f2f,0x702f,0x712f,0x722f,0x732f,0x742f,0x752f,0x762f,
0x772f,0x782f,0x792f,0x7a2f,0x302f,0x312f,0x322f,0x332f,0x342f,0x352f,0x362f,0x372f,0x382f,0x392f,0x2b2f,0x2f2f };
 
#define EU32(_u_) (lut2[(_u_ >>  8) & 0xfff] << 16 |\
                   lut2[ _u_ >> 20])
                             
#define EI32(_i_) {\
  unsigned _u = BSWAP32(ux); ux = ctou32(ip+6+_i_*6  );\
  unsigned _v = BSWAP32(vx); vx = ctou32(ip+6+_i_*6+3);\
  _u = EU32(_u); _v = EU32(_v); \
  ctou32(op+_i_*8) = _u; ctou32(op+_i_*8+4) = _v; \
}                  

unsigned tb64xenc(const unsigned char *in, unsigned inlen, unsigned char *out) {
  static unsigned char lut1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const  unsigned char *ip = in;
         unsigned char *op = out;
         unsigned      outlen = TURBOB64LEN(inlen);
  
  if(outlen >= 128+4) {
    unsigned ux = ctou32(ip  ),
             vx = ctou32(ip+3);
    for(; op <= out+(outlen-(128+4)); op += 128, ip += (128/4)*3) { // unrolling 96 bytes
      EI32(0); EI32(1); EI32( 2); EI32( 3); EI32( 4); EI32( 5); EI32( 6); EI32( 7);      
      EI32(8); EI32(9); EI32(10); EI32(11); EI32(12); EI32(13); EI32(14); EI32(15);      PREFETCH(ip,256, 0);
    }
  }
  ETAIL();
  return outlen;
}

