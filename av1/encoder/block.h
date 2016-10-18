/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef AV1_ENCODER_BLOCK_H_
#define AV1_ENCODER_BLOCK_H_

#include "av1/common/entropymv.h"
#include "av1/common/entropy.h"
#if CONFIG_REF_MV
#include "av1/common/mvref_common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned int sse;
  int sum;
  unsigned int var;
} diff;

typedef struct macroblock_plane {
  DECLARE_ALIGNED(16, int16_t, src_diff[MAX_SB_SQUARE]);
  tran_low_t *qcoeff;
  tran_low_t *coeff;
  uint16_t *eobs;
  struct buf_2d src;

  // Quantizer setings
  const int16_t *quant_fp;
  const int16_t *round_fp;
  const int16_t *quant;
  const int16_t *quant_shift;
  const int16_t *zbin;
  const int16_t *round;
#if CONFIG_NEW_QUANT
  const cuml_bins_type_nuq *cuml_bins_nuq[QUANT_PROFILES];
#endif  // CONFIG_NEW_QUANT

  int64_t quant_thred[2];
} MACROBLOCK_PLANE;

/* The [2] dimension is for whether we skip the EOB node (i.e. if previous
 * coefficient in this block was zero) or not. */
typedef unsigned int av1_coeff_cost[PLANE_TYPES][REF_TYPES][COEF_BANDS][2]
                                   [COEFF_CONTEXTS][ENTROPY_TOKENS];

typedef struct {
  int_mv ref_mvs[MODE_CTX_REF_FRAMES][MAX_MV_REF_CANDIDATES];
  int16_t mode_context[MODE_CTX_REF_FRAMES];
#if CONFIG_REF_MV
  uint8_t ref_mv_count[MODE_CTX_REF_FRAMES];
  CANDIDATE_MV ref_mv_stack[MODE_CTX_REF_FRAMES][MAX_REF_MV_STACK_SIZE];
#if CONFIG_EXT_INTER
  int16_t compound_mode_context[MODE_CTX_REF_FRAMES];
#endif  // CONFIG_EXT_INTER
#endif
} MB_MODE_INFO_EXT;

#if CONFIG_PALETTE
typedef struct {
  uint8_t best_palette_color_map[MAX_SB_SQUARE];
  float kmeans_data_buf[2 * MAX_SB_SQUARE];
} PALETTE_BUFFER;
#endif  // CONFIG_PALETTE

typedef struct macroblock MACROBLOCK;
struct macroblock {
  struct macroblock_plane plane[MAX_MB_PLANE];

  MACROBLOCKD e_mbd;
  MB_MODE_INFO_EXT *mbmi_ext;
  int skip_block;
  int select_tx_size;
  int skip_optimize;
  int q_index;

  // The equivalent error at the current rdmult of one whole bit (not one
  // bitcost unit).
  int errorperbit;
  // The equivalend SAD error of one (whole) bit at the current quantizer
  // for large blocks.
  int sadperbit16;
  // The equivalend SAD error of one (whole) bit at the current quantizer
  // for sub-8x8 blocks.
  int sadperbit4;
  int rddiv;
  int rdmult;
  int mb_energy;
  int *m_search_count_ptr;
  int *ex_search_count_ptr;

  // These are set to their default values at the beginning, and then adjusted
  // further in the encoding process.
  BLOCK_SIZE min_partition_size;
  BLOCK_SIZE max_partition_size;

  int mv_best_ref_index[TOTAL_REFS_PER_FRAME];
  unsigned int max_mv_context[TOTAL_REFS_PER_FRAME];
  unsigned int source_variance;
  unsigned int recon_variance;
  unsigned int pred_sse[TOTAL_REFS_PER_FRAME];
  int pred_mv_sad[TOTAL_REFS_PER_FRAME];

#if CONFIG_REF_MV
  int *nmvjointcost;
  int nmv_vec_cost[NMV_CONTEXTS][MV_JOINTS];
  int *nmvcost[NMV_CONTEXTS][2];
  int *nmvcost_hp[NMV_CONTEXTS][2];
  int **mv_cost_stack[NMV_CONTEXTS];
  int *nmvjointsadcost;
#else
  int nmvjointcost[MV_JOINTS];
  int *nmvcost[2];
  int *nmvcost_hp[2];
  int nmvjointsadcost[MV_JOINTS];
#endif

  int **mvcost;
  int *nmvsadcost[2];
  int *nmvsadcost_hp[2];
  int **mvsadcost;
#if CONFIG_MOTION_VAR
  int32_t *wsrc_buf;
  int32_t *mask_buf;
#endif  // CONFIG_MOTION_VAR

#if CONFIG_PALETTE
  PALETTE_BUFFER *palette_buffer;
#endif  // CONFIG_PALETTE

  // These define limits to motion vector components to prevent them
  // from extending outside the UMV borders
  int mv_col_min;
  int mv_col_max;
  int mv_row_min;
  int mv_row_max;

#if CONFIG_VAR_TX
  uint8_t blk_skip[MAX_MB_PLANE][MAX_MIB_SIZE * MAX_MIB_SIZE * 4];
#if CONFIG_REF_MV
  uint8_t blk_skip_drl[MAX_MB_PLANE][MAX_MIB_SIZE * MAX_MIB_SIZE * 4];
#endif
#endif

  int skip;

  int encode_breakout;

  // note that token_costs is the cost when eob node is skipped
  av1_coeff_cost token_costs[TX_SIZES];

  int optimize;

  // indicate if it is in the rd search loop or encoding process
  int use_lp32x32fdct;

  // Used to store sub partition's choices.
  MV pred_mv[TOTAL_REFS_PER_FRAME];

  // Store the best motion vector during motion search
  int_mv best_mv;
  // Store the second best motion vector during full-pixel motion search
  int_mv second_best_mv;

  // use default transform and skip transform type search for intra modes
  int use_default_intra_tx_type;
  // use default transform and skip transform type search for inter modes
  int use_default_inter_tx_type;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // AV1_ENCODER_BLOCK_H_