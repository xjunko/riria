#include <riria/types.h>
#include <riria/ubsan.h>
#include <stdint.h>

uint64_t __stack_chk_guard = 0xB00B5;

// clang-format off
__attribute__((noreturn)) 
void __stack_chk_fail(void) {
  panic("stack smashing detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_pointer_overflow(void) {
    panic("pointer overflow detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_type_mismatch_v1(struct type_mismatch_v1_data *data, uintptr_t ptr) {
    if (ptr == 0) printf("null pointer is being used! \n");
    printf("happens at %s:%u:%u\n", data->location.file, data->location.line, data->location.column);
    panic("type mismatch detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_function_type_mismatch(void) {
    panic("function type mismatch detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_load_invalid_value(void) {
    panic("load invalid value detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_divrem_overflow(void) {
    panic("division/remainder overflow detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_shift_out_of_bounds(void) {
    panic("shift out of bounds detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_out_of_bounds(void) {
    panic("out of bounds detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_add_overflow(void) {
    panic("addition overflow detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_sub_overflow(void) {
    panic("subtraction overflow detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_mul_overflow(void) {
    panic("multiplication overflow detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_negate_overflow(void) {
    panic("negation overflow detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_invalid_builtin(void) {
    panic("invalid builtin detected");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_builtin_unreachable(void) {
    panic("unreachable builtin executed");
}
// clang-format on
