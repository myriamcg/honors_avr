#include <unistd.h>
#include "frida-gumjs.h"

#include "config.h"

#include "instrument.h"
#include "persistent.h"
#include "util.h"

#if defined(__aarch64__)
typedef struct {

  GumCpuContext ctx;
  uint64_t      rflags;

} persistent_ctx_t;

static persistent_ctx_t saved_regs = {0};
static gpointer         persistent_loop = NULL;

gboolean persistent_is_supported(void) {

  return true;

}

static void instrument_persitent_save_regs(GumArm64Writer   *cw,
                                           persistent_ctx_t *regs) {

  GumAddress    regs_address = GUM_ADDRESS(regs);
  const guint32 mrs_x1_nzcv = 0xd53b4201;

  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X0, ARM64_REG_X1, ARM64_REG_SP, -(16 + GUM_RED_ZONE_SIZE),
      GUM_INDEX_PRE_ADJUST);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(cw, ARM64_REG_X2, ARM64_REG_X3,
                                              ARM64_REG_SP, -(16),
                                              GUM_INDEX_PRE_ADJUST);

  gum_arm64_writer_put_instruction(cw, mrs_x1_nzcv);

  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X0,
                                       GUM_ADDRESS(regs_address));

  /* Skip x0 & x1 we'll do that later */

  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X2, ARM64_REG_X3, ARM64_REG_X0,
      offsetof(GumCpuContext, x[2]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X4, ARM64_REG_X5, ARM64_REG_X0,
      offsetof(GumCpuContext, x[4]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X6, ARM64_REG_X7, ARM64_REG_X0,
      offsetof(GumCpuContext, x[6]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X8, ARM64_REG_X9, ARM64_REG_X0,
      offsetof(GumCpuContext, x[8]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X10, ARM64_REG_X11, ARM64_REG_X0,
      offsetof(GumCpuContext, x[10]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X12, ARM64_REG_X13, ARM64_REG_X0,
      offsetof(GumCpuContext, x[12]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X14, ARM64_REG_X15, ARM64_REG_X0,
      offsetof(GumCpuContext, x[14]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X16, ARM64_REG_X17, ARM64_REG_X0,
      offsetof(GumCpuContext, x[16]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X18, ARM64_REG_X19, ARM64_REG_X0,
      offsetof(GumCpuContext, x[18]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X20, ARM64_REG_X21, ARM64_REG_X0,
      offsetof(GumCpuContext, x[20]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X22, ARM64_REG_X23, ARM64_REG_X0,
      offsetof(GumCpuContext, x[22]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X24, ARM64_REG_X25, ARM64_REG_X0,
      offsetof(GumCpuContext, x[24]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X26, ARM64_REG_X27, ARM64_REG_X0,
      offsetof(GumCpuContext, x[26]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X28, ARM64_REG_X29, ARM64_REG_X0,
      offsetof(GumCpuContext, x[28]), GUM_INDEX_SIGNED_OFFSET);

  /* LR (x30) */
  gum_arm64_writer_put_str_reg_reg_offset(cw, ARM64_REG_X30, ARM64_REG_X0,
                                          offsetof(GumCpuContext, lr));

  /* PC & Adjusted SP (31) */
  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X2,
                                       GUM_ADDRESS(persistent_start));
  gum_arm64_writer_put_add_reg_reg_imm(cw, ARM64_REG_X3, ARM64_REG_SP,
                                       (GUM_RED_ZONE_SIZE + 32));
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X2, ARM64_REG_X3, ARM64_REG_X0, offsetof(GumCpuContext, pc),
      GUM_INDEX_SIGNED_OFFSET);

  /* CPSR */
  gum_arm64_writer_put_str_reg_reg_offset(cw, ARM64_REG_X1, ARM64_REG_X0,
                                          offsetof(persistent_ctx_t, rflags));

  /* Q */
  for (int i = 0; i < 16; i++) {

    gum_arm64_writer_put_stp_reg_reg_reg_offset(
        cw, ARM64_REG_Q0 + (i * 2), ARM64_REG_Q0 + (i * 2) + 1, ARM64_REG_X0,
        offsetof(GumCpuContext, v[i]), GUM_INDEX_SIGNED_OFFSET);

  }

  /* x0 & x1 */
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(cw, ARM64_REG_X2, ARM64_REG_X3,
                                              ARM64_REG_SP, 16,
                                              GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X2, ARM64_REG_X3, ARM64_REG_X0,
      offsetof(GumCpuContext, x[0]), GUM_INDEX_SIGNED_OFFSET);

  /* Pop the saved values */
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X2, ARM64_REG_X3, ARM64_REG_SP, 16, GUM_INDEX_POST_ADJUST);

  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X0, ARM64_REG_X1, ARM64_REG_SP, 16 + GUM_RED_ZONE_SIZE,
      GUM_INDEX_POST_ADJUST);

}

static void instrument_persitent_restore_regs(GumArm64Writer   *cw,
                                              persistent_ctx_t *regs) {

  GumAddress    regs_address = GUM_ADDRESS(regs);
  const guint32 msr_nzcv_x1 = 0xd51b4201;

  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X0,
                                       GUM_ADDRESS(regs_address));

  /* Skip x0 - x3 we'll do that last */

  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X4, ARM64_REG_X5, ARM64_REG_X0,
      offsetof(GumCpuContext, x[4]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X6, ARM64_REG_X7, ARM64_REG_X0,
      offsetof(GumCpuContext, x[6]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X8, ARM64_REG_X9, ARM64_REG_X0,
      offsetof(GumCpuContext, x[8]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X10, ARM64_REG_X11, ARM64_REG_X0,
      offsetof(GumCpuContext, x[10]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X12, ARM64_REG_X13, ARM64_REG_X0,
      offsetof(GumCpuContext, x[12]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X14, ARM64_REG_X15, ARM64_REG_X0,
      offsetof(GumCpuContext, x[14]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X16, ARM64_REG_X17, ARM64_REG_X0,
      offsetof(GumCpuContext, x[16]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X18, ARM64_REG_X19, ARM64_REG_X0,
      offsetof(GumCpuContext, x[18]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X20, ARM64_REG_X21, ARM64_REG_X0,
      offsetof(GumCpuContext, x[20]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X22, ARM64_REG_X23, ARM64_REG_X0,
      offsetof(GumCpuContext, x[22]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X24, ARM64_REG_X25, ARM64_REG_X0,
      offsetof(GumCpuContext, x[24]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X26, ARM64_REG_X27, ARM64_REG_X0,
      offsetof(GumCpuContext, x[26]), GUM_INDEX_SIGNED_OFFSET);
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X28, ARM64_REG_X29, ARM64_REG_X0,
      offsetof(GumCpuContext, x[28]), GUM_INDEX_SIGNED_OFFSET);

  /* LR (x30) */
  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X30, ARM64_REG_X0,
                                          offsetof(GumCpuContext, lr));

  /* Adjusted SP (31) (use x1 as clobber)*/
  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X1, ARM64_REG_X0,
                                          offsetof(GumCpuContext, sp));
  gum_arm64_writer_put_mov_reg_reg(cw, ARM64_REG_SP, ARM64_REG_X1);

  /* CPSR */
  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X1, ARM64_REG_X0,
                                          offsetof(persistent_ctx_t, rflags));
  gum_arm64_writer_put_instruction(cw, msr_nzcv_x1);

  /* Q */
  for (int i = 0; i < 16; i++) {

    gum_arm64_writer_put_ldp_reg_reg_reg_offset(
        cw, ARM64_REG_Q0 + (i * 2), ARM64_REG_Q0 + (i * 2) + 1, ARM64_REG_X0,
        offsetof(GumCpuContext, v[i]), GUM_INDEX_SIGNED_OFFSET);

  }

  /* x2 & x3 */
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X2, ARM64_REG_X3, ARM64_REG_X0,
      offsetof(GumCpuContext, x[2]), GUM_INDEX_SIGNED_OFFSET);
  /* x0 & x1 */
  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X0, ARM64_REG_X1, ARM64_REG_X0,
      offsetof(GumCpuContext, x[0]), GUM_INDEX_SIGNED_OFFSET);

}

static void instrument_afl_persistent_loop_func(void) {

  if (__afl_persistent_loop(persistent_count) == 0) { _exit(0); }

  if (instrument_previous_pc_addr == NULL) {

    FATAL("instrument_previous_pc_addr uninitialized");

  }

  *instrument_previous_pc_addr = instrument_hash_zero;

}

static void instrument_afl_persistent_loop(GumArm64Writer *cw) {

  gum_arm64_writer_put_sub_reg_reg_imm(cw, ARM64_REG_SP, ARM64_REG_SP,
                                       GUM_RED_ZONE_SIZE);
  gum_arm64_writer_put_call_address_with_arguments(
      cw, GUM_ADDRESS(instrument_afl_persistent_loop_func), 0);
  gum_arm64_writer_put_add_reg_reg_imm(cw, ARM64_REG_SP, ARM64_REG_SP,
                                       GUM_RED_ZONE_SIZE);

}

static void persistent_prologue_hook(GumArm64Writer   *cw,
                                     persistent_ctx_t *regs) {

  if (persistent_hook == NULL) return;

  gum_arm64_writer_put_sub_reg_reg_imm(cw, ARM64_REG_SP, ARM64_REG_SP,
                                       GUM_RED_ZONE_SIZE);
  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X2,
                                       GUM_ADDRESS(&__afl_fuzz_len));
  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X2, ARM64_REG_X2, 0);
  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X2, ARM64_REG_X2, 0);

  gum_arm64_writer_put_mov_reg_reg(cw, ARM64_REG_W2, ARM64_REG_W2);

  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X1,
                                       GUM_ADDRESS(&__afl_fuzz_ptr));
  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X1, ARM64_REG_X1, 0);

  gum_arm64_writer_put_call_address_with_arguments(
      cw, GUM_ADDRESS(persistent_hook), 3, GUM_ARG_ADDRESS,
      GUM_ADDRESS(&regs->ctx), GUM_ARG_REGISTER, ARM64_REG_X1, GUM_ARG_REGISTER,
      ARM64_REG_X2);

  gum_arm64_writer_put_add_reg_reg_imm(cw, ARM64_REG_SP, ARM64_REG_SP,
                                       GUM_RED_ZONE_SIZE);

}

static void instrument_persitent_save_lr(GumArm64Writer *cw) {

  gum_arm64_writer_put_stp_reg_reg_reg_offset(
      cw, ARM64_REG_X0, ARM64_REG_X1, ARM64_REG_SP, -(16 + GUM_RED_ZONE_SIZE),
      GUM_INDEX_PRE_ADJUST);

  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X0,
                                       GUM_ADDRESS(&persistent_ret));

  gum_arm64_writer_put_str_reg_reg_offset(cw, ARM64_REG_LR, ARM64_REG_X0, 0);

  gum_arm64_writer_put_ldp_reg_reg_reg_offset(
      cw, ARM64_REG_X0, ARM64_REG_X1, ARM64_REG_SP, 16 + GUM_RED_ZONE_SIZE,
      GUM_INDEX_POST_ADJUST);

}

void persistent_prologue_arch(GumStalkerOutput *output) {

  /*
   *  SAVE RET (Used to write the epilogue if persistent_ret is not set)
   *  SAVE REGS
   * loop: (Save address of where the eiplogue should jump back to)
   *  CALL instrument_afl_persistent_loop
   *  CALL hook (optionally)
   *  RESTORE REGS
   *  CALL INSTRUMENTED PERSISTENT FUNC
   *  JMP loop
   *  INSTRUMENTED PERSISTENT FUNC
   */

  GumArm64Writer *cw = output->writer.arm64;

  FVERBOSE("Persistent loop reached");

  /* This is the location our epilogue should be written below */
  if (persistent_ret == 0) { instrument_persitent_save_lr(cw); }

  /* Save the current context */
  instrument_persitent_save_regs(cw, &saved_regs);

  /*
   * Store a pointer to where we should return for our next iteration.
   * This is the location our epilogue should branch to
   */
  persistent_loop = gum_arm64_writer_cur(cw);

  gconstpointer loop = cw->code + 1;
  gum_arm64_writer_put_label(cw, loop);

  /*
   * call __afl_persistent_loop which will _exit if we have reached our
   * loop count. Also reset our previous_pc
   */
  instrument_afl_persistent_loop(cw);

  /* Optionally call the persistent hook */
  persistent_prologue_hook(cw, &saved_regs);

  /* Restore our CPU context before we continue execution */
  instrument_persitent_restore_regs(cw, &saved_regs);

  gconstpointer original = cw->code + 1;

  /*
   * Call our original code, that way we regain control if our target
   * function returns without reaching the epilogue as an additional
   * safety net
   */
  gum_arm64_writer_put_bl_label(cw, original);

  /*
   * Return for our next iteration if our original function returns
   * and control hasn't reached the epilogue for some reason
   */
  gum_arm64_writer_put_b_label(cw, loop);

  /*
   * The original code for our target function will be emitted
   * immediately following this
   */
  gum_arm64_writer_put_label(cw, original);

  if (persistent_debug) { gum_arm64_writer_put_brk_imm(cw, 0); }

}

void persistent_epilogue_arch(GumStalkerOutput *output) {

  GumArm64Writer *cw = output->writer.arm64;

  if (persistent_debug) { gum_arm64_writer_put_brk_imm(cw, 0); }

  gum_arm64_writer_put_ldr_reg_address(cw, ARM64_REG_X0,
                                       GUM_ADDRESS(&persistent_loop));

  gum_arm64_writer_put_ldr_reg_reg_offset(cw, ARM64_REG_X0, ARM64_REG_X0, 0);

  gum_arm64_writer_put_br_reg(cw, ARM64_REG_X0);

}

#endif

