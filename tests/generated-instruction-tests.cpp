#include "ppc_context.h"
PPC_EXTERN_FUNC(shared_exit);
__attribute__((alias("__imp__test_vaddsws_3"))) PPC_WEAK_FUNC(test_vaddsws_3);
PPC_FUNC_IMPL(__imp__test_vaddsws_3) {
	PPC_FUNC_PROLOGUE();
	// vaddsws v3,v4,v5
	ctx.v3 = ppc_vaddsws(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vaddsws_4"))) PPC_WEAK_FUNC(test_vaddsws_4);
PPC_FUNC_IMPL(__imp__test_vaddsws_4) {
	PPC_FUNC_PROLOGUE();
	// vaddsws v4,v4,v5
	ctx.v4 = ppc_vaddsws(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vaddsws_5"))) PPC_WEAK_FUNC(test_vaddsws_5);
PPC_FUNC_IMPL(__imp__test_vaddsws_5) {
	PPC_FUNC_PROLOGUE();
	// vaddsws v5,v4,v5
	ctx.v5 = ppc_vaddsws(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vadduhs_3"))) PPC_WEAK_FUNC(test_vadduhs_3);
PPC_FUNC_IMPL(__imp__test_vadduhs_3) {
	PPC_FUNC_PROLOGUE();
	// vadduhs v3,v4,v5
	ctx.v3 = ppc_vadduhs(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vadduhs_4"))) PPC_WEAK_FUNC(test_vadduhs_4);
PPC_FUNC_IMPL(__imp__test_vadduhs_4) {
	PPC_FUNC_PROLOGUE();
	// vadduhs v4,v4,v5
	ctx.v4 = ppc_vadduhs(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vadduhs_5"))) PPC_WEAK_FUNC(test_vadduhs_5);
PPC_FUNC_IMPL(__imp__test_vadduhs_5) {
	PPC_FUNC_PROLOGUE();
	// vadduhs v5,v4,v5
	ctx.v5 = ppc_vadduhs(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vcmpgtuw_3"))) PPC_WEAK_FUNC(test_vcmpgtuw_3);
PPC_FUNC_IMPL(__imp__test_vcmpgtuw_3) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v3,v4,v5
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v5.u32[0] ? UINT32_MAX : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v5.u32[1] ? UINT32_MAX : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v5.u32[2] ? UINT32_MAX : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v5.u32[3] ? UINT32_MAX : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__test_vcmpgtuw_4"))) PPC_WEAK_FUNC(test_vcmpgtuw_4);
PPC_FUNC_IMPL(__imp__test_vcmpgtuw_4) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v4,v4,v5
	ctx.v4.u32[0] = ctx.v4.u32[0] > ctx.v5.u32[0] ? UINT32_MAX : 0;
	ctx.v4.u32[1] = ctx.v4.u32[1] > ctx.v5.u32[1] ? UINT32_MAX : 0;
	ctx.v4.u32[2] = ctx.v4.u32[2] > ctx.v5.u32[2] ? UINT32_MAX : 0;
	ctx.v4.u32[3] = ctx.v4.u32[3] > ctx.v5.u32[3] ? UINT32_MAX : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__test_vcmpgtuw_5"))) PPC_WEAK_FUNC(test_vcmpgtuw_5);
PPC_FUNC_IMPL(__imp__test_vcmpgtuw_5) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v5,v4,v5
	ctx.v5.u32[0] = ctx.v4.u32[0] > ctx.v5.u32[0] ? UINT32_MAX : 0;
	ctx.v5.u32[1] = ctx.v4.u32[1] > ctx.v5.u32[1] ? UINT32_MAX : 0;
	ctx.v5.u32[2] = ctx.v4.u32[2] > ctx.v5.u32[2] ? UINT32_MAX : 0;
	ctx.v5.u32[3] = ctx.v4.u32[3] > ctx.v5.u32[3] ? UINT32_MAX : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__test_vminsw_3"))) PPC_WEAK_FUNC(test_vminsw_3);
PPC_FUNC_IMPL(__imp__test_vminsw_3) {
	PPC_FUNC_PROLOGUE();
	// vminsw v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.u32), simde_mm_load_si128((simde__m128i*)ctx.v5.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__test_vminsw_4"))) PPC_WEAK_FUNC(test_vminsw_4);
PPC_FUNC_IMPL(__imp__test_vminsw_4) {
	PPC_FUNC_PROLOGUE();
	// vminsw v4,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v4.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.u32), simde_mm_load_si128((simde__m128i*)ctx.v5.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__test_vminsw_5"))) PPC_WEAK_FUNC(test_vminsw_5);
PPC_FUNC_IMPL(__imp__test_vminsw_5) {
	PPC_FUNC_PROLOGUE();
	// vminsw v5,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v5.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.u32), simde_mm_load_si128((simde__m128i*)ctx.v5.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__test_vpkswss128_3"))) PPC_WEAK_FUNC(test_vpkswss128_3);
PPC_FUNC_IMPL(__imp__test_vpkswss128_3) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v3,v4,v5
	ctx.v3 = ppc_vpkswss(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vpkswss128_4"))) PPC_WEAK_FUNC(test_vpkswss128_4);
PPC_FUNC_IMPL(__imp__test_vpkswss128_4) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v4,v4,v5
	ctx.v4 = ppc_vpkswss(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vpkswss128_5"))) PPC_WEAK_FUNC(test_vpkswss128_5);
PPC_FUNC_IMPL(__imp__test_vpkswss128_5) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v5,v4,v5
	ctx.v5 = ppc_vpkswss(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vnor128_3"))) PPC_WEAK_FUNC(test_vnor128_3);
PPC_FUNC_IMPL(__imp__test_vnor128_3) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v3,v4,v5
	ctx.v3.u64[0] = ~(ctx.v4.u64[0] | ctx.v5.u64[0]);
	ctx.v3.u64[1] = ~(ctx.v4.u64[1] | ctx.v5.u64[1]);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vnor128_4"))) PPC_WEAK_FUNC(test_vnor128_4);
PPC_FUNC_IMPL(__imp__test_vnor128_4) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v4,v4,v5
	ctx.v4.u64[0] = ~(ctx.v4.u64[0] | ctx.v5.u64[0]);
	ctx.v4.u64[1] = ~(ctx.v4.u64[1] | ctx.v5.u64[1]);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vnor128_5"))) PPC_WEAK_FUNC(test_vnor128_5);
PPC_FUNC_IMPL(__imp__test_vnor128_5) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v5,v4,v5
	ctx.v5.u64[0] = ~(ctx.v4.u64[0] | ctx.v5.u64[0]);
	ctx.v5.u64[1] = ~(ctx.v4.u64[1] | ctx.v5.u64[1]);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vpkuwus128_3"))) PPC_WEAK_FUNC(test_vpkuwus128_3);
PPC_FUNC_IMPL(__imp__test_vpkuwus128_3) {
	PPC_FUNC_PROLOGUE();
	// vpkuwus128 v3,v4,v5
	ctx.v3 = ppc_vpkuwus(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vpkuwus128_4"))) PPC_WEAK_FUNC(test_vpkuwus128_4);
PPC_FUNC_IMPL(__imp__test_vpkuwus128_4) {
	PPC_FUNC_PROLOGUE();
	// vpkuwus128 v4,v4,v5
	ctx.v4 = ppc_vpkuwus(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vpkuwus128_5"))) PPC_WEAK_FUNC(test_vpkuwus128_5);
PPC_FUNC_IMPL(__imp__test_vpkuwus128_5) {
	PPC_FUNC_PROLOGUE();
	// vpkuwus128 v5,v4,v5
	ctx.v5 = ppc_vpkuwus(ctx.v4, ctx.v5, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vsro128_3"))) PPC_WEAK_FUNC(test_vsro128_3);
PPC_FUNC_IMPL(__imp__test_vsro128_3) {
	PPC_FUNC_PROLOGUE();
	// vsro128 v3,v4,v5
	ctx.v3 = ppc_vsro(ctx.v4, ctx.v5);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vsro128_4"))) PPC_WEAK_FUNC(test_vsro128_4);
PPC_FUNC_IMPL(__imp__test_vsro128_4) {
	PPC_FUNC_PROLOGUE();
	// vsro128 v4,v4,v5
	ctx.v4 = ppc_vsro(ctx.v4, ctx.v5);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vsro128_5"))) PPC_WEAK_FUNC(test_vsro128_5);
PPC_FUNC_IMPL(__imp__test_vsro128_5) {
	PPC_FUNC_PROLOGUE();
	// vsro128 v5,v4,v5
	ctx.v5 = ppc_vsro(ctx.v4, ctx.v5);
	// blr 
	return;
}

__attribute__((alias("__imp__test_vcmpgtuw_record"))) PPC_WEAK_FUNC(test_vcmpgtuw_record);
PPC_FUNC_IMPL(__imp__test_vcmpgtuw_record) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v3,v4,v5
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v5.u32[0] ? UINT32_MAX : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v5.u32[1] ? UINT32_MAX : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v5.u32[2] ? UINT32_MAX : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v5.u32[3] ? UINT32_MAX : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__test_convert_0_3"))) PPC_WEAK_FUNC(test_convert_0_3);
PPC_FUNC_IMPL(__imp__test_convert_0_3) {
	PPC_FUNC_PROLOGUE();
	// vcfpuxws128 v3,v5,0
	ctx.fpscr.enableFlushMode();
	ctx.v3 = ppc_vctuxs(ctx.v5, 0, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_convert_0_5"))) PPC_WEAK_FUNC(test_convert_0_5);
PPC_FUNC_IMPL(__imp__test_convert_0_5) {
	PPC_FUNC_PROLOGUE();
	// vcfpuxws128 v5,v5,0
	ctx.fpscr.enableFlushMode();
	ctx.v5 = ppc_vctuxs(ctx.v5, 0, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_convert_2_3"))) PPC_WEAK_FUNC(test_convert_2_3);
PPC_FUNC_IMPL(__imp__test_convert_2_3) {
	PPC_FUNC_PROLOGUE();
	// vcfpuxws128 v3,v5,2
	ctx.fpscr.enableFlushMode();
	ctx.v3 = ppc_vctuxs(ctx.v5, 2, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_convert_2_5"))) PPC_WEAK_FUNC(test_convert_2_5);
PPC_FUNC_IMPL(__imp__test_convert_2_5) {
	PPC_FUNC_PROLOGUE();
	// vcfpuxws128 v5,v5,2
	ctx.fpscr.enableFlushMode();
	ctx.v5 = ppc_vctuxs(ctx.v5, 2, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_convert_31_3"))) PPC_WEAK_FUNC(test_convert_31_3);
PPC_FUNC_IMPL(__imp__test_convert_31_3) {
	PPC_FUNC_PROLOGUE();
	// vcfpuxws128 v3,v5,31
	ctx.fpscr.enableFlushMode();
	ctx.v3 = ppc_vctuxs(ctx.v5, 31, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_convert_31_5"))) PPC_WEAK_FUNC(test_convert_31_5);
PPC_FUNC_IMPL(__imp__test_convert_31_5) {
	PPC_FUNC_PROLOGUE();
	// vcfpuxws128 v5,v5,31
	ctx.fpscr.enableFlushMode();
	ctx.v5 = ppc_vctuxs(ctx.v5, 31, ctx.vscr);
	// blr 
	return;
}

__attribute__((alias("__imp__test_eqv"))) PPC_WEAK_FUNC(test_eqv);
PPC_FUNC_IMPL(__imp__test_eqv) {
	PPC_FUNC_PROLOGUE();
	// eqv r3,r4,r5
	ctx.r3.u64 = ~(ctx.r4.u64 ^ ctx.r5.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__test_eqv_record"))) PPC_WEAK_FUNC(test_eqv_record);
PPC_FUNC_IMPL(__imp__test_eqv_record) {
	PPC_FUNC_PROLOGUE();
	// eqv. r3,r4,r5
	ctx.r3.u64 = ~(ctx.r4.u64 ^ ctx.r5.u64);
	ctx.cr0.compare<int32_t>(ctx.r3.s32, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__test_stfsu"))) PPC_WEAK_FUNC(test_stfsu);
PPC_FUNC_IMPL(__imp__test_stfsu) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsu f3,8(r4)
	ctx.fpscr.disableFlushMode();
	ea = ctx.r4.u32 + 8;
	temp.f32 = float(ctx.f3.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__test_lvebx"))) PPC_WEAK_FUNC(test_lvebx);
PPC_FUNC_IMPL(__imp__test_lvebx) {
	PPC_FUNC_PROLOGUE();
	// lvebx v3,r4,r5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_shuffle_epi8(simde_mm_load_si128((simde__m128i*)(base + ((ctx.r4.u32 + ctx.r5.u32) & ~0xF))), simde_mm_load_si128((simde__m128i*)VectorMaskL)));
	// blr 
	return;
}

__attribute__((alias("__imp__test_lvebx_ra0"))) PPC_WEAK_FUNC(test_lvebx_ra0);
PPC_FUNC_IMPL(__imp__test_lvebx_ra0) {
	PPC_FUNC_PROLOGUE();
	// lvebx v3,r0,r5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_shuffle_epi8(simde_mm_load_si128((simde__m128i*)(base + ((ctx.r5.u32) & ~0xF))), simde_mm_load_si128((simde__m128i*)VectorMaskL)));
	// blr 
	return;
}

__attribute__((alias("__imp__test_dcbst"))) PPC_WEAK_FUNC(test_dcbst);
PPC_FUNC_IMPL(__imp__test_dcbst) {
	PPC_FUNC_PROLOGUE();
	// dcbst r4,r5
	// blr 
	return;
}

__attribute__((alias("__imp__test_computed"))) PPC_WEAK_FUNC(test_computed);
PPC_FUNC_IMPL(__imp__test_computed) {
	PPC_FUNC_PROLOGUE();
loc_1000:
	// nop 
loc_1004:
	// bctr 
	switch (ctx.ctr.u32 & ~3u) {
	case 0x1000: goto loc_1000;
	case 0x1004: goto loc_1004;
	case 0x1008: goto loc_1008;
	case 0x100C: goto loc_100C;
	case 0x1010: goto loc_1010;
	case 0x1014: goto loc_1014;
	default: PPC_CALL_INDIRECT_FUNC(ctx.ctr.u32 & ~3u); return;
	}
loc_1008:
	// li r3,7
	ctx.r3.s64 = 7;
loc_100C:
	// blr 
	return;
loc_1010:
	// li r3,9
	ctx.r3.s64 = 9;
loc_1014:
	// blr 
	return;
}

__attribute__((alias("__imp__test_external"))) PPC_WEAK_FUNC(test_external);
PPC_FUNC_IMPL(__imp__test_external) {
	PPC_FUNC_PROLOGUE();
	// nop 
	// bctr 
	switch (ctx.r3.u64) {
	case 0:
		shared_exit(ctx, base);
		return;
	default:
		__builtin_unreachable();
	}
}

