#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/array.h"

static void
mrb_exec_at_exit(mrb_state *mrb)
{
  mrb_value stack = mrb_obj_iv_get(mrb, (struct RObject*)mrb->kernel_module, mrb_intern_lit(mrb, "__at_exit_stack__"));
  mrb_int i;
  struct RObject *saved_exc;
  mrb_value result;

  if (!mrb_array_p(stack)) {
    return;
  }

  /*
  Main exception was already printed on the here.
  But riase this error again on mrb_funcall if mrb->exc setted.
  So, Avoid by mrb->exc = NULL.
  */
  saved_exc = mrb->exc;
  mrb->exc = NULL;
  for (i = RARRAY_LEN(stack); 0 < i; i--) {
    result = mrb_funcall(mrb, mrb_ary_ref(mrb, stack, i - 1), "call", 0, NULL);
    if (mrb_type(result) == MRB_TT_EXCEPTION) {
      mrb_print_error(mrb);
      mrb->exc = NULL; /* Avoid before exception */
    }
  }
  mrb->exc = saved_exc;
}

/*
 *  call-seq:
 *     at_exit { block } -> proc
 *
 *  Converts _block_ to a +Proc+ object (and therefore
 *  binds it at the point of call) and registers it for execution when
 *  the program exits. If multiple handlers are registered, they are
 *  executed in reverse order of registration.
 *
 *     def do_at_exit(str1)
 *       at_exit { print str1 }
 *     end
 *     at_exit { puts "cruel world" }
 *     do_at_exit("goodbye ")
 *     exit
 *
 *  <em>produces:</em>
 *
 *     goodbye cruel world
 */
static mrb_value
mrb_f_at_exit(mrb_state *mrb, mrb_value krn)
{
  mrb_value block = mrb_nil_value();
  mrb_value callbacks = mrb_obj_iv_get(mrb, (struct RObject*)mrb->kernel_module, mrb_intern_lit(mrb, "__at_exit_stack__"));

  mrb_get_args(mrb, "&", &block);
  if (mrb_nil_p(block)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "called without a block");
  }
  if (mrb_nil_p(callbacks)) {
    callbacks = mrb_ary_new(mrb);
    mrb_obj_iv_set(mrb, (struct RObject*)mrb->kernel_module, mrb_intern_lit(mrb, "__at_exit_stack__"), callbacks);
  }
  mrb_ary_push(mrb, callbacks, block);
  return block;
}

void
mrb_mruby_at_exit_gem_init(mrb_state* mrb)
{
  struct RClass *krn = mrb->kernel_module;
  mrb_define_module_function(mrb, krn, "at_exit", mrb_f_at_exit, MRB_ARGS_NONE() | MRB_ARGS_BLOCK());
}

void
mrb_mruby_at_exit_gem_final(mrb_state* mrb)
{
  mrb_exec_at_exit(mrb);
}
