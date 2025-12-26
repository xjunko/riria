#pragma once
#include <riria/types.h>

struct source_location {
  const char* file;
  uint32_t line;
  uint32_t column;
};

struct type_descriptor {
  uint16_t kind;
  uint16_t info;
  char name[];
};

struct overflow_data {
  struct source_location location;
  struct type_descriptor* type;
};

struct shift_out_of_bounds_data {
  struct source_location location;
  struct type_descriptor* left_type;
  struct type_descriptor* right_type;
};

struct invalid_value_data {
  struct source_location location;
  struct type_descriptor* type;
};

struct array_out_of_bounds_data {
  struct source_location location;
  struct type_descriptor* array_type;
  struct type_descriptor* index_type;
};

struct type_mismatch_v1_data {
  struct source_location location;
  struct type_descriptor* type;
  uint8_t log_alignment;
  uint8_t type_check_kind;
};

struct negative_vla_data {
  struct source_location location;
  struct type_descriptor* type;
};

struct nonnull_return_data {
  struct source_location location;
};

struct nonnull_arg_data {
  struct source_location location;
};

struct unreachable_data {
  struct source_location location;
};

struct invalid_builtin_data {
  struct source_location location;
  uint8_t kind;
};

struct float_cast_overflow_data {
  struct source_location location;
};

struct missing_return_data {
  struct source_location location;
};

struct alignment_assumption_data {
  struct source_location location;
  struct source_location assumption_location;
  struct type_descriptor* type;
};