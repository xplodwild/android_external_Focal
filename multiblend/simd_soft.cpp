#include <stdint.h>

typedef struct {
	union {
		uint64_t u64[2];
		int64_t  s64[2];
		uint32_t u32[4];
		int32_t  s32[4];
		uint16_t u16[8];
		int16_t  s16[8];
		uint8_t  u8[16];
		int8_t   s8[16];
	};
} __m128i;

__m128i _mm_slli_epi32(__m128i a, int count)
{
	for(int i = 0; i < 4; i++)
		a.u32[i] <<= count;

	return a;
}

__m128i _mm_srai_epi32(__m128i a, int count)
{
	for(int i = 0; i < 4; i++)
		a.s32[i] >>= count;

	return a;
}

__m128i _mm_srai_epi16(__m128i a, int count)
{
	for(int i = 0; i < 8; i++)
		a.s16[i] >>= count;

	return a;
}


__m128i _mm_add_epi32(__m128i a, __m128i b)
{
	for(int i = 0; i < 4; i++)
		a.s32[i]+=b.s32[i];

	return a;
}

__m128i _mm_add_epi16(__m128i a, __m128i b)
{
	for(int i = 0; i < 8; i++)
		a.s16[i]+=b.s16[i];

	return a;
}

__m128i _mm_sub_epi32(__m128i a, __m128i b)
{
	for(int i = 0; i < 4; i++)
		a.s32[i]-=b.s32[i];

	return a;
}

__m128i _mm_sub_epi16(__m128i a, __m128i b)
{
	for(int i = 0; i < 8; i++)
		a.s16[i]-=b.s16[i];

	return a;
}
