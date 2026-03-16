/* Copyright (C) 2026  gemmaro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* FilePos, Message, and MessageIterator each hold a @file instance
 * variable (ivar) pointing to the GettextPO::File object that owns
 * the underlying po_file_t.  This prevents the File object from being
 * garbage collected while any of these objects are still alive, which
 * would otherwise cause their po_file_t pointers to become dangling.
 * When allocating one of these objects, @file must be propagated from
 * the parent object: File -> MessageIterator -> Message -> FilePos.
 */

#include "gettextpo.h"
#include <gettext-po.h>
#include <ruby/internal/core/rdata.h>
#include <ruby/internal/core/rstring.h>
#include <ruby/internal/error.h>
#include <ruby/internal/globals.h>
#include <ruby/internal/intern/string.h>
#include <ruby/internal/module.h>
#include <ruby/internal/scan_args.h>
#include <ruby/internal/special_consts.h>

#define ERROR                                                                 \
  rb_const_get (rb_const_get (rb_cObject, rb_intern ("GettextPO")),           \
                rb_intern ("Error"))

VALUE rb_cMessage;
VALUE rb_cMessageIterator;
VALUE rb_cFilePos;
VALUE rb_cFlagIterator;

/* ********** error ********** */

static struct
{
  bool error;
  VALUE *user_xerror;
  VALUE *user_xerror2;
  VALUE *file;
} gettextpo_xerror_context = {};

static void
gettextpo_xerror_context_reset (void)
{
  gettextpo_xerror_context.error = false;
  gettextpo_xerror_context.user_xerror = NULL;
  gettextpo_xerror_context.user_xerror2 = NULL;
  gettextpo_xerror_context.file = NULL;
}

static void
gettextpo_xerror (const int severity, const po_message_t message,
                  const char *const filename, const size_t lineno,
                  const size_t column, const int multiline_p,
                  const char *const message_text)
{
  gettextpo_xerror_context.error = true;
  if (!gettextpo_xerror_context.user_xerror
      || NIL_P (*gettextpo_xerror_context.user_xerror))
    return;
  VALUE kwargs = rb_hash_new ();
  rb_hash_aset (kwargs, ID2SYM (rb_intern ("severity")), INT2NUM (severity));
  if (message)
    {
      VALUE message_value = rb_obj_alloc (rb_cMessage);
      DATA_PTR (message_value) = message;
      rb_ivar_set (message_value, rb_intern ("@file"),
                   *gettextpo_xerror_context.file);
      rb_hash_aset (kwargs, ID2SYM (rb_intern ("message")), message_value);
    }
  if (filename)
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("filename")),
                  rb_str_new_cstr (filename));
  if (filename && (lineno != (size_t)(-1)))
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("lineno")), INT2NUM (lineno));
  if (filename && lineno != (size_t)(-1) && column != (size_t)(-1))
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("column")), INT2NUM (column));
  rb_hash_aset (kwargs, ID2SYM (rb_intern ("multiline")),
                multiline_p ? Qtrue : Qfalse);
  if (message_text)
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("message_text")),
                  rb_str_new_cstr (message_text));
  VALUE args = rb_ary_new ();
  rb_ary_push (args, kwargs);
  rb_proc_call_kw (*gettextpo_xerror_context.user_xerror, args,
                   RB_PASS_KEYWORDS);
  if (severity == PO_SEVERITY_FATAL_ERROR)
    abort ();
}

static void
gettextpo_xerror2 (const int severity, const po_message_t message1,
                   const char *const filename1, const size_t lineno1,
                   const size_t column1, const int multiline_p1,
                   const char *const message_text1,
                   const po_message_t message2, const char *const filename2,
                   const size_t lineno2, const size_t column2,
                   const int multiline_p2, const char *const message_text2)
{
  gettextpo_xerror_context.error = true;
  if (!gettextpo_xerror_context.user_xerror2
      || NIL_P (*gettextpo_xerror_context.user_xerror2))
    return;
  VALUE kwargs = rb_hash_new ();
  rb_hash_aset (kwargs, ID2SYM (rb_intern ("severity")), INT2NUM (severity));
  if (message1)
    {
      VALUE message_value1 = rb_obj_alloc (rb_cMessage);
      DATA_PTR (message_value1) = message1;
      rb_ivar_set (message_value1, rb_intern ("@file"),
                   *gettextpo_xerror_context.file);
      rb_hash_aset (kwargs, ID2SYM (rb_intern ("message1")), message_value1);
    }
  if (filename1)
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("filename1")),
                  rb_str_new_cstr (filename1));
  if (filename1 && (lineno1 != (size_t)(-1)))
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("lineno1")), INT2NUM (lineno1));
  if (filename1 && lineno1 != (size_t)(-1) && column1 != (size_t)(-1))
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("column1")), INT2NUM (column1));
  rb_hash_aset (kwargs, ID2SYM (rb_intern ("multiline1")),
                multiline_p1 ? Qtrue : Qfalse);
  if (message_text1)
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("message_text1")),
                  rb_str_new_cstr (message_text1));
  if (message2)
    {
      VALUE message_value2 = rb_obj_alloc (rb_cMessage);
      DATA_PTR (message_value2) = message2;
      rb_ivar_set (message_value2, rb_intern ("@file"),
                   *gettextpo_xerror_context.file);
      rb_hash_aset (kwargs, ID2SYM (rb_intern ("message2")), message_value2);
    }
  if (filename2)
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("filename2")),
                  rb_str_new_cstr (filename2));
  if (filename2 && (lineno2 != (size_t)(-1)))
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("lineno2")), INT2NUM (lineno2));
  if (filename2 && lineno2 != (size_t)(-1) && column2 != (size_t)(-1))
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("column2")), INT2NUM (column2));
  rb_hash_aset (kwargs, ID2SYM (rb_intern ("multiline2")),
                multiline_p2 ? Qtrue : Qfalse);
  if (message_text2)
    rb_hash_aset (kwargs, ID2SYM (rb_intern ("message_text2")),
                  rb_str_new_cstr (message_text2));
  VALUE args = rb_ary_new ();
  rb_ary_push (args, kwargs);
  rb_proc_call_kw (*gettextpo_xerror_context.user_xerror2, args,
                   RB_PASS_KEYWORDS);
  if (severity == PO_SEVERITY_FATAL_ERROR)
    abort ();
}

static const struct po_xerror_handler gettextpo_xerror_handler = {
  .xerror = gettextpo_xerror,
  .xerror2 = gettextpo_xerror2,
};

#ifdef HAVE_PO_MESSAGE_GET_FORMAT
/**
 * Document-class: GettextPO::FlagIterator
 */

void
gettextpo_flag_iterator_free (void *iter)
{
  if (iter)
    po_flag_iterator_free (iter);
}

static const rb_data_type_t gettextpo_po_flag_iterator_type = {
  .wrap_struct_name = "gettextpo PO flag iterator",
  .function = {
    .dfree = gettextpo_flag_iterator_free,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
gettextpo_po_flag_iterator_alloc (VALUE self)
{
  return TypedData_Wrap_Struct (self, &gettextpo_po_flag_iterator_type, NULL);
}

static VALUE
gettextpo_po_flag_iterator_m_next (VALUE self)
{
  const char *flag = po_flag_next (DATA_PTR (self));
  if (flag)
    return rb_str_new_cstr (flag);
  else
    rb_raise (rb_eStopIteration, "no more flag");
}
#endif

/**
 * Document-class: GettextPO::Message
 */

static const rb_data_type_t gettextpo_po_message_type = {
  .wrap_struct_name = "gettextpo PO message",
  .function = {
    .dfree = RUBY_TYPED_NEVER_FREE,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
gettextpo_po_message_alloc (VALUE self)
{
  return TypedData_Wrap_Struct (self, &gettextpo_po_message_type, NULL);
}

#define OPTIONAL_STRING_GETTER(field)                                         \
  VALUE                                                                       \
  gettextpo_po_message_m_##field (VALUE self)                                 \
  {                                                                           \
    const char *string = po_message_##field (DATA_PTR (self));                \
    return string ? rb_str_new_cstr (string) : Qnil;                          \
  }
#define OPTIONAL_STRING_SETTER(field)                                         \
  VALUE                                                                       \
  gettextpo_po_message_m_##field##_set (VALUE self, VALUE string)             \
  {                                                                           \
    po_message_set_##field (                                                  \
        DATA_PTR (self), NIL_P (string) ? NULL : StringValueCStr (string));   \
    return Qnil;                                                              \
  }

/**
 * Document-method: msgctxt
 *
 * Possibly returns +nil+.
 */
OPTIONAL_STRING_GETTER (msgctxt);

/**
 * Document-method: msgctxt=
 * call-seq: msgctxt = context
 *
 * +context+ can be +nil+ to remove it.
 */
OPTIONAL_STRING_SETTER (msgctxt);

#define STRING_GETTER(field)                                                  \
  VALUE                                                                       \
  gettextpo_po_message_m_##field (VALUE self)                                 \
  {                                                                           \
    return rb_str_new_cstr (po_message_##field (DATA_PTR (self)));            \
  }
#define STRING_SETTER(field)                                                  \
  VALUE                                                                       \
  gettextpo_po_message_m_##field##_set (VALUE self, VALUE string)             \
  {                                                                           \
    po_message_set_##field (DATA_PTR (self), StringValueCStr (string));       \
    return Qnil;                                                              \
  }

/**
 * Document-method: msgid
 */
STRING_GETTER (msgid);

/**
 * Document-method: msgid=
 * call-seq: msgid = id
 */
STRING_SETTER (msgid);

/**
 * Document-method: msgid_plural
 *
 * Possibly returns +nil+.
 */
OPTIONAL_STRING_GETTER (msgid_plural);

/**
 * Document-method: msgid_plural=
 * call-seq: msgid_plural = id
 *
 * +id+ can be +nil+ to remove it.
 */
OPTIONAL_STRING_SETTER (msgid_plural);

/**
 * Document-method: msgstr
 */
STRING_GETTER (msgstr);

/**
 * Document-method: msgstr=
 * call-seq: msgstr = str
 */
STRING_SETTER (msgstr);

/**
 * call-seq: msgstr_plural (index)
 *
 * Possibly returns +nil+.
 */
VALUE
gettextpo_po_message_m_msgstr_plural (VALUE self, VALUE index)
{
  const char *msgstr_plural
      = po_message_msgstr_plural (DATA_PTR (self), NUM2INT (index));
  return msgstr_plural ? rb_str_new_cstr (msgstr_plural) : Qnil;
}

/**
 * call-seq: set_msgstr_plural (index, str)
 *
 * +str+ can be +nil+ to remove it.
 */
VALUE
gettextpo_po_message_m_msgstr_plural_set (VALUE self, VALUE index,
                                          VALUE msgstr_plural)
{
  po_message_set_msgstr_plural (
      DATA_PTR (self), NUM2INT (index),
      NIL_P (msgstr_plural) ? NULL : StringValueCStr (msgstr_plural));
  return Qnil;
}

/**
 * Document-method: comments
 */
STRING_GETTER (comments);

/**
 * Document-method: comments=
 * call-seq: comments = str
 */
STRING_SETTER (comments);

/**
 * Document-method: extracted_comments
 */
STRING_GETTER (extracted_comments);

/**
 * Document-method: extracted_comments=
 * call-seq: extracted_comments = str
 */
STRING_SETTER (extracted_comments);

/**
 * Document-method: prev_msgctxt
 *
 * Possibly returns +nil+.
 */
OPTIONAL_STRING_GETTER (prev_msgctxt);

/**
 * Document-method: prev_msgctxt=
 * call-seq: prev_msgctxt = context
 *
 * +context+ can be +nil+ to remove it.
 */
OPTIONAL_STRING_SETTER (prev_msgctxt);

/**
 * Document-method: prev_msgid
 *
 * Possibly returns +nil+.
 */
OPTIONAL_STRING_GETTER (prev_msgid);

/**
 * Document-method: prev_msgid=
 * call-seq: prev_msgid = id
 *
 * +id+ can be +nil+ to remove it.
 */
OPTIONAL_STRING_SETTER (prev_msgid);

/**
 * Document-method: prev_msgid_plural
 *
 * Possibly returns +nil+.
 */
OPTIONAL_STRING_GETTER (prev_msgid_plural);

/**
 * Document-method: prev_msgid_plural=
 * call-seq: prev_msgid_plural = id
 *
 * +id+ can be +nil+ to remove it.
 */
OPTIONAL_STRING_SETTER (prev_msgid_plural);

#define BOOL_GETTER(field)                                                    \
  VALUE                                                                       \
  gettextpo_po_message_m_##field (VALUE self)                                 \
  {                                                                           \
    return po_message_is_##field (DATA_PTR (self)) ? Qtrue : Qfalse;          \
  }
#define BOOL_SETTER(field)                                                    \
  VALUE                                                                       \
  gettextpo_po_message_m_##field##_set (VALUE self, VALUE boolean)            \
  {                                                                           \
    po_message_set_##field (DATA_PTR (self), RB_TEST (boolean));              \
    return Qnil;                                                              \
  }

/**
 * Document-method: obsolete?
 */
BOOL_GETTER (obsolete);

/**
 * Document-method: obsolete=
 * call-seq: obsolete = bool
 */
BOOL_SETTER (obsolete);

/**
 * Document-method: fuzzy?
 */
BOOL_GETTER (fuzzy);

/**
 * Document-method: fuzzy=
 * call-seq: fuzzy = bool
 */
BOOL_SETTER (fuzzy);

#ifdef HAVE_PO_MESSAGE_GET_FORMAT
#define FLAG_Q(name)                                                          \
  static VALUE gettextpo_po_message_m_##name##_flag_q (VALUE self,            \
                                                       VALUE flag)            \
  {                                                                           \
    return po_message_has_##name##_flag (DATA_PTR (self),                     \
                                         StringValueCStr (flag))              \
               ? Qtrue                                                        \
               : Qfalse;                                                      \
  }
#define UPDATE_FLAG(name)                                                     \
  static VALUE gettextpo_po_message_m_update_##name##_flag (                  \
      int argc, VALUE *argv, VALUE self)                                      \
  {                                                                           \
    VALUE flag, kwargs;                                                       \
    rb_scan_args (argc, argv, "1:", &flag, &kwargs);                          \
    ID kwargs_ids[] = { rb_intern ("set") };                                  \
    VALUE kwargs_vals[] = { Qundef };                                         \
    rb_get_kwargs (kwargs, kwargs_ids, 0,                                     \
                   sizeof (kwargs_ids) / sizeof (kwargs_ids[0]),              \
                   kwargs_vals);                                              \
                                                                              \
    po_message_set_##name##_flag (DATA_PTR (self), StringValueCStr (flag),    \
                                  RB_UNDEF_P (kwargs_vals[0])                 \
                                      && RB_TEST (kwargs_vals[0]));           \
    return Qnil;                                                              \
  }
#define FLAG_ITER(name)                                                       \
  static VALUE gettextpo_po_message_m_##name##_flags_iterator (VALUE self)    \
  {                                                                           \
    VALUE iter = rb_obj_alloc (rb_cFlagIterator);                             \
    DATA_PTR (iter) = po_message_##name##_flags_iterator (DATA_PTR (self));   \
    return iter;                                                              \
  }

/**
 * Document-method: workflow_flag?
 * call-seq: workflow_flag? (flag)
 */
FLAG_Q (workflow);

/**
 * Document-method: update_workflow_flag
 * call-seq: update_workflow_flag (flag, set: true)
 */
UPDATE_FLAG (workflow);

/**
 * Document-method: workflow_flag_iterator
 */
FLAG_ITER (workflow);

/**
 * call-seq: format (type)
 *
 * Returns +true+, +false+ (opposite), or +nil+ (none).
 */
static VALUE
gettextpo_po_message_m_format (VALUE self, VALUE format)
{
  int result
      = po_message_get_format (DATA_PTR (self), StringValueCStr (format));
  switch (result)
    {
    case 1:
      return Qtrue;
      break;
    case 0:
      return Qfalse;
      break;
    case -1:
      return Qnil;
      break;
    default:
      rb_raise (ERROR, "unreachable");
    }
}
#endif

/**
 * call-seq: format? (type)
 */
VALUE
gettextpo_po_message_m_format_q (VALUE self, VALUE format)
{
  return po_message_is_format (DATA_PTR (self), StringValueCStr (format))
             ? Qtrue
             : Qfalse;
}

/**
 * call-seq: update_format (type, opposite: false, remove: false)
 *
 * Set by default.  +opposite+ sets a "no-" prefixed format type.
 */
VALUE
gettextpo_po_message_m_format_set (int argc, VALUE *argv, VALUE self)
{
  VALUE format, kwargs;
  rb_scan_args (argc, argv, "1:", &format, &kwargs);
  ID kwargs_ids[] = { rb_intern ("opposite"), rb_intern ("remove") };
  VALUE kwargs_vals[] = { Qundef, Qundef };
  rb_get_kwargs (kwargs, kwargs_ids, 0, 2, kwargs_vals);
  bool opposite = !RB_UNDEF_P (kwargs_vals[0]) && RB_TEST (kwargs_vals[0]);
  bool remove = !RB_UNDEF_P (kwargs_vals[1]) && RB_TEST (kwargs_vals[1]);
  if (opposite && remove)
    rb_raise (ERROR, "opposite and remove cannot be set at the same time");
  po_message_set_format (DATA_PTR (self), StringValueCStr (format),
                         opposite ? 0 : (remove ? -1 : 1));
  return Qnil;
}

#ifdef HAVE_PO_MESSAGE_GET_FORMAT
/**
 * Document-method: sticky_flag?
 * call-seq: sticky_flag? (flag)
 */
FLAG_Q (sticky);

/**
 * Document-method: update_sticky_flag
 * call-seq: update_sticky_flag (flag, set: true)
 */
UPDATE_FLAG (sticky);

/**
 * Document-method: sticky_flag_iterator
 */
FLAG_ITER (sticky);
#endif

/**
 * call-seq: range? (range)
 */
VALUE
gettextpo_po_message_m_range (VALUE self, VALUE range)
{
  VALUE min, max;
  int exclude;
  rb_range_values (range, &min, &max, &exclude);
  int min_int = NUM2INT (min);
  int max_int = NUM2INT (max);
  return po_message_is_range (DATA_PTR (self), &min_int, &max_int) ? Qtrue
                                                                   : Qfalse;
}

/**
 * call-seq: range = range
 */
VALUE
gettextpo_po_message_m_range_set (VALUE self, VALUE range)
{
  VALUE min, max;
  int exclude;
  rb_range_values (range, &min, &max, &exclude);
  po_message_set_range (DATA_PTR (self), min, max);
  return Qnil;
}

/**
 * call-seq: filepos (index) -> GettextPO::FilePos
 *
 * Possibly returns +nil+.
 */
VALUE
gettextpo_po_message_m_filepos (VALUE self, VALUE index)
{
  po_filepos_t pos = po_message_filepos (DATA_PTR (self), NUM2INT (index));
  if (pos)
    {
      VALUE filepos = rb_obj_alloc (rb_cFilePos);
      DATA_PTR (filepos) = pos;
      rb_ivar_set (filepos, rb_intern ("@file"),
                   rb_ivar_get (self, rb_intern ("@file")));
      return filepos;
    }
  else
    return Qnil;
}

/**
 * call-seq: remove_filepos (index)
 */
VALUE
gettextpo_po_message_m_remove_filepos (VALUE self, VALUE index)
{
  po_message_remove_filepos (DATA_PTR (self), NUM2INT (index));
  return Qnil;
}

/**
 * call-seq: add_filepos (file, start_line)
 */
VALUE
gettextpo_po_message_m_add_filepos (VALUE self, VALUE file, VALUE start_line)
{
  po_message_add_filepos (DATA_PTR (self), StringValueCStr (file),
                          NUM2INT (start_line));
  return Qnil;
}

/**
 * call-seq: check_all (iterator, xerror: nil, xerror2: nil)
 *
 * See also GettextPO::File.read for exception error handlings.
 */
VALUE
gettextpo_po_message_m_check_all (int argc, VALUE *argv, VALUE self)
{
  VALUE iterator, kwargs;
  rb_scan_args (argc, argv, "1:", &iterator, &kwargs);
  ID kwargs_ids[] = { rb_intern ("xerror"), rb_intern ("xerror2") };
  VALUE kwargs_vals[] = { Qundef, Qundef };
  rb_get_kwargs (kwargs, kwargs_ids, 0, 2, kwargs_vals);
  gettextpo_xerror_context_reset ();
  VALUE file = rb_ivar_get (self, rb_intern ("@file"));
  gettextpo_xerror_context.file = &file;
  if (kwargs_vals[0] != Qundef)
    gettextpo_xerror_context.user_xerror = &kwargs_vals[0];
  if (kwargs_vals[1] != Qundef)
    gettextpo_xerror_context.user_xerror2 = &kwargs_vals[1];
  po_message_check_all (DATA_PTR (self), DATA_PTR (iterator),
                        &gettextpo_xerror_handler);
  if (gettextpo_xerror_context.error)
    rb_raise (ERROR, "check all for message failed");
  return Qnil;
}

/**
 * call-seq: check_format (xerror: nil, xerror2: nil)
 *
 * See also GettextPO::File.read for exception error handlings.
 */
VALUE
gettextpo_po_message_m_check_format (int argc, VALUE *argv, VALUE self)
{
  VALUE kwargs;
  rb_scan_args (argc, argv, ":", &kwargs);
  ID kwargs_ids[] = { rb_intern ("xerror"), rb_intern ("xerror2") };
  VALUE kwargs_vals[] = { Qundef, Qundef };
  rb_get_kwargs (kwargs, kwargs_ids, 0, 2, kwargs_vals);
  gettextpo_xerror_context_reset ();
  VALUE file = rb_ivar_get (self, rb_intern ("@file"));
  gettextpo_xerror_context.file = &file;
  if (kwargs_vals[0] != Qundef)
    gettextpo_xerror_context.user_xerror = &kwargs_vals[0];
  if (kwargs_vals[1] != Qundef)
    gettextpo_xerror_context.user_xerror2 = &kwargs_vals[1];
  po_message_check_format (DATA_PTR (self), &gettextpo_xerror_handler);
  if (gettextpo_xerror_context.error)
    rb_raise (ERROR, "check format for message failed");
  return Qnil;
}

/* ********** file ********** */

void
gettextpo_file_free (void *file)
{
  if (file)
    po_file_free (file);
}

static const rb_data_type_t gettextpo_po_file_type = {
  .wrap_struct_name = "gettextpo PO file",
  .function = {
    .dfree = gettextpo_file_free,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
gettextpo_po_file_alloc (VALUE self)
{
  return TypedData_Wrap_Struct (self, &gettextpo_po_file_type, NULL);
}

/**
 * See also #message_iterator and GettextPO::MessageIterator#insert
 * methods for the further manipulations.
 */
VALUE
gettextpo_po_file_m_initialize (VALUE self)
{
  DATA_PTR (self) = po_file_create ();
  return self;
}

/**
 * call-seq: read (filename, xerror: nil, xerror2: nil) -> GettextPO::File
 *
 * +xerror+ and +xerror2+ are +Proc+ objects.  +xerror+ takes keyword
 * arguments +severity+, +message+, +filename+, +lineno+, +column+,
 * +multiline+, and +message_text+.  +xerror2+ takes keyword arguments
 * +severity+, +message1+, +filename1+, +lineno1+, +column1+,
 * +multiline1+, +message_text1+, +message2+, +filename2+, +lineno2+,
 * +column2+, +multiline2+, and +message_text2+.  See also GettextPO
 * for general exception handlings.
 */
VALUE
gettextpo_po_file_m_read (int argc, VALUE *argv, VALUE klass)
{
  VALUE filename, kwargs;
  rb_scan_args (argc, argv, "1:", &filename, &kwargs);
  ID kwargs_ids[] = { rb_intern ("xerror"), rb_intern ("xerror2") };
  VALUE kwargs_vals[] = { Qundef, Qundef };
  rb_get_kwargs (kwargs, kwargs_ids, 0, 2, kwargs_vals);
  gettextpo_xerror_context_reset ();
  VALUE self = rb_obj_alloc (klass);
  gettextpo_xerror_context.file = &self;
  if (kwargs_vals[0] != Qundef)
    gettextpo_xerror_context.user_xerror = &kwargs_vals[0];
  if (kwargs_vals[1] != Qundef)
    gettextpo_xerror_context.user_xerror2 = &kwargs_vals[1];
  po_file_t file
      = po_file_read (StringValueCStr (filename), &gettextpo_xerror_handler);
  if (file)
    {
      DATA_PTR (self) = file;
      if (gettextpo_xerror_context.error)
        rb_raise (ERROR, "failed to read");
      return self;
    }
  else
    rb_raise (ERROR, "failed to read file, maybe file not found?");
}

/**
 * call-seq: write (filename, xerror: nil, xerror2: nil)
 *
 * See also ::read for exception error handlings.
 */
VALUE
gettextpo_po_file_m_write (int argc, VALUE *argv, VALUE self)
{
  VALUE filename, kwargs;
  rb_scan_args (argc, argv, "1:", &filename, &kwargs);
  ID kwargs_ids[] = { rb_intern ("xerror"), rb_intern ("xerror2") };
  VALUE kwargs_vals[] = { Qundef, Qundef };
  rb_get_kwargs (kwargs, kwargs_ids, 0, 2, kwargs_vals);
  gettextpo_xerror_context_reset ();
  gettextpo_xerror_context.file = &self;
  if (kwargs_vals[0] != Qundef)
    gettextpo_xerror_context.user_xerror = &kwargs_vals[0];
  if (kwargs_vals[1] != Qundef)
    gettextpo_xerror_context.user_xerror2 = &kwargs_vals[1];
  po_file_write (DATA_PTR (self), StringValueCStr (filename),
                 &gettextpo_xerror_handler);
  if (gettextpo_xerror_context.error)
    rb_raise (ERROR, "failed to write");
  return Qnil;
}

VALUE
gettextpo_po_file_m_domains (VALUE self)
{
  const char *const *domains = po_file_domains (DATA_PTR (self));
  VALUE result = rb_ary_new ();
  for (size_t index = 0; domains[index]; index++)
    rb_ary_push (result, rb_str_new_cstr (domains[index]));
  return result;
}

/**
 * call-seq: message_iterator (domain = nil) -> GettextPO::MessageIterator
 *
 * You may find it handy to use the #each_message method instead of
 * this to iterate over the messages.  This method is for more
 * versatile manipulation; the returned iterator can insert messages,
 * for example.
 */
VALUE
gettextpo_po_file_m_message_iterator (int argc, VALUE *argv, VALUE self)
{
  VALUE domain;
  rb_scan_args (argc, argv, "01", &domain);
  VALUE iterator = rb_obj_alloc (rb_cMessageIterator);
  DATA_PTR (iterator) = po_message_iterator (
      DATA_PTR (self), NIL_P (domain) ? NULL : StringValueCStr (domain));
  rb_ivar_set (iterator, rb_intern ("@file"), self);
  return iterator;
}

/**
 * call-seq: domain_header (domain = nil)
 *
 * +domain+ can be +nil+ to use a default.  Possibly returns +nil+.
 *
 * See also GettextPO.header_entry_value.
 */
VALUE
gettextpo_po_file_m_domain_header (int argc, VALUE *argv, VALUE self)
{
  VALUE domain;
  rb_scan_args (argc, argv, "01", &domain);
  const char *header = po_file_domain_header (
      DATA_PTR (self), NIL_P (domain) ? NULL : StringValueCStr (domain));
  return header ? rb_str_new_cstr (header) : Qnil;
}

/**
 * call-seq: check_all (xerror: nil, xerror2: nil)
 *
 * See also ::read for exception error handlings.
 */
VALUE
gettextpo_po_file_m_check_all (int argc, VALUE *argv, VALUE self)
{
  VALUE kwargs;
  rb_scan_args (argc, argv, ":", &kwargs);
  ID kwargs_ids[] = { rb_intern ("xerror"), rb_intern ("xerror2") };
  VALUE kwargs_vals[] = { Qundef, Qundef };
  rb_get_kwargs (kwargs, kwargs_ids, 0, 2, kwargs_vals);
  gettextpo_xerror_context_reset ();
  gettextpo_xerror_context.file = &self;
  if (kwargs_vals[0] != Qundef)
    gettextpo_xerror_context.user_xerror = &kwargs_vals[0];
  if (kwargs_vals[1] != Qundef)
    gettextpo_xerror_context.user_xerror2 = &kwargs_vals[1];
  po_file_check_all (DATA_PTR (self), &gettextpo_xerror_handler);
  if (gettextpo_xerror_context.error)
    rb_raise (ERROR, "check all for file failed");
  return Qnil;
}

/* ********** iterator ********** */

static const rb_data_type_t gettextpo_po_message_iterator_type = {
  .wrap_struct_name = "gettextpo PO message iterator",
  .function = {
    .dfree = (void (*)(void *)) po_message_iterator_free,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
gettextpo_po_message_iterator_alloc (VALUE self)
{
  return TypedData_Wrap_Struct (self, &gettextpo_po_message_iterator_type,
                                NULL);
}

/**
 * call-seq: next -> GettextPO::Message
 */
VALUE
gettextpo_po_message_iterator_m_next (VALUE self)
{
  po_message_t message = po_next_message (DATA_PTR (self));
  if (message)
    {
      VALUE message_value = rb_obj_alloc (rb_cMessage);
      DATA_PTR (message_value) = message;
      rb_ivar_set (message_value, rb_intern ("@file"),
                   rb_ivar_get (self, rb_intern ("@file")));
      return message_value;
    }
  else
    rb_raise (rb_eStopIteration, "end of PO message iterator");
}

/**
 * call-seq: insert (msgid, msgstr) -> GettextPO::Message
 */
VALUE
gettextpo_po_message_iterator_m_insert (VALUE self, VALUE msgid, VALUE msgstr)
{
  po_message_t message = po_message_create ();
  po_message_set_msgid (message, StringValueCStr (msgid));
  po_message_set_msgstr (message, StringValueCStr (msgstr));
  po_message_insert (DATA_PTR (self), message);
  VALUE value = rb_obj_alloc (rb_cMessage);
  DATA_PTR (value) = message;
  rb_ivar_set (value, rb_intern ("@file"),
               rb_ivar_get (self, rb_intern ("@file")));
  return value;
}

/**
 * Document-class: GettextPO::FilePos
 */

static const rb_data_type_t gettextpo_po_message_filepos_type = {
  .wrap_struct_name = "gettextpo PO message filepos",
  .function = {
    .dfree = RUBY_TYPED_NEVER_FREE,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
gettextpo_po_filepos_alloc (VALUE self)
{
  return TypedData_Wrap_Struct (self, &gettextpo_po_message_filepos_type,
                                NULL);
}

/**
 * Possibly returns +nil+.
 */
VALUE
gettextpo_po_filepos_m_file (VALUE self)
{
  const char *file = po_filepos_file (DATA_PTR (self));
  return file ? rb_str_new_cstr (file) : Qnil;
}

/**
 * Possibly returns +nil+.
 */
VALUE
gettextpo_po_filepos_m_start_line (VALUE self)
{
  size_t start_line = po_filepos_start_line (DATA_PTR (self));
  return start_line == (size_t)(-1) ? Qnil : INT2NUM (start_line);
}

/* ********** others ********** */

/**
 * call-seq: header_entry_value (header, field)
 *
 * See also GettextPO::File#domain_header.
 */
VALUE
gettextpo_m_header_entry_value (VALUE self, VALUE header, VALUE field)
{
  char *string
      = po_header_field (StringValueCStr (header), StringValueCStr (field));
  if (string)
    {
      VALUE value = rb_str_new_cstr (string);
      free (string);
      return value;
    }
  else
    return Qnil;
}

/**
 * call-seq: header_with_updated_entry_value (header, field, value)
 *
 * See also GettextPO::File#domain_header.
 */
VALUE
gettextpo_m_header_with_updated_entry_value (VALUE self, VALUE header,
                                             VALUE field, VALUE value)
{
  char *updated_header
      = po_header_set_field (StringValueCStr (header), StringValueCStr (field),
                             StringValueCStr (value));
  VALUE header_value = rb_str_new_cstr (updated_header);
  free (updated_header);
  return header_value;
}

VALUE
gettextpo_po_format_list (VALUE self)
{
  VALUE list = rb_ary_new ();
  const char *const *formats = po_format_list ();
  for (size_t index = 0; formats[index]; index++)
    rb_ary_push (list, rb_str_new_cstr (formats[index]));
  return list;
}

/**
 * call-seq: format_pretty_name (format)
 *
 * Possibly returns +nil+.
 *
 * See also ::formats method for available format types.
 */
VALUE
gettextpo_po_format_pretty_name (VALUE self, VALUE format)
{
  const char *name = po_format_pretty_name (StringValueCStr (format));
  return name ? rb_str_new_cstr (name) : Qnil;
}

/* ********** init ********** */

RUBY_FUNC_EXPORTED void
Init_gettextpo (void)
{
  VALUE rb_mGettextPO = rb_define_module ("GettextPO");
  rb_define_const (rb_mGettextPO, "LIBGETTEXTPO_VERSION",
                   INT2NUM (libgettextpo_version));
  rb_define_const (rb_mGettextPO, "SEVERITY_WARNING",
                   INT2NUM (PO_SEVERITY_WARNING));
  rb_define_const (rb_mGettextPO, "SEVERITY_ERROR",
                   INT2NUM (PO_SEVERITY_ERROR));
  rb_define_const (rb_mGettextPO, "SEVERITY_FATAL_ERROR",
                   INT2NUM (PO_SEVERITY_FATAL_ERROR));
  rb_define_singleton_method (rb_mGettextPO, "header_entry_value",
                              gettextpo_m_header_entry_value, 2);
  rb_define_singleton_method (rb_mGettextPO, "header_with_updated_entry_value",
                              gettextpo_m_header_with_updated_entry_value, 3);
  rb_define_singleton_method (rb_mGettextPO, "formats",
                              gettextpo_po_format_list, 0);
  rb_define_singleton_method (rb_mGettextPO, "format_pretty_name",
                              gettextpo_po_format_pretty_name, 1);
  VALUE rb_cFile = rb_define_class_under (rb_mGettextPO, "File", rb_cObject);
  rb_define_alloc_func (rb_cFile, gettextpo_po_file_alloc);
  rb_define_singleton_method (rb_cFile, "read", gettextpo_po_file_m_read, -1);
  rb_define_method (rb_cFile, "initialize", gettextpo_po_file_m_initialize, 0);
  rb_define_method (rb_cFile, "write", gettextpo_po_file_m_write, -1);
  rb_define_method (rb_cFile, "domains", gettextpo_po_file_m_domains, 0);
  rb_define_method (rb_cFile, "message_iterator",
                    gettextpo_po_file_m_message_iterator, -1);
  rb_define_method (rb_cFile, "domain_header",
                    gettextpo_po_file_m_domain_header, -1);
  rb_define_method (rb_cFile, "check_all", gettextpo_po_file_m_check_all, -1);
  rb_cMessage = rb_define_class_under (rb_mGettextPO, "Message", rb_cObject);
  rb_define_alloc_func (rb_cMessage, gettextpo_po_message_alloc);
  rb_define_method (rb_cMessage, "msgctxt", gettextpo_po_message_m_msgctxt, 0);
  rb_define_method (rb_cMessage,
                    "msgctxt=", gettextpo_po_message_m_msgctxt_set, 1);
  rb_define_method (rb_cMessage, "msgid", gettextpo_po_message_m_msgid, 0);
  rb_define_method (rb_cMessage, "msgid=", gettextpo_po_message_m_msgid_set,
                    1);
  rb_define_method (rb_cMessage, "msgid_plural",
                    gettextpo_po_message_m_msgid_plural, 0);
  rb_define_method (rb_cMessage,
                    "msgid_plural=", gettextpo_po_message_m_msgid_plural_set,
                    1);
  rb_define_method (rb_cMessage, "msgstr", gettextpo_po_message_m_msgstr, 0);
  rb_define_method (rb_cMessage, "msgstr=", gettextpo_po_message_m_msgstr_set,
                    1);
  rb_define_method (rb_cMessage, "msgstr_plural",
                    gettextpo_po_message_m_msgstr_plural, 1);
  rb_define_method (rb_cMessage, "set_msgstr_plural",
                    gettextpo_po_message_m_msgstr_plural_set, 2);
  rb_define_method (rb_cMessage, "comments", gettextpo_po_message_m_comments,
                    0);
  rb_define_method (rb_cMessage,
                    "comments=", gettextpo_po_message_m_comments_set, 1);
  rb_define_method (rb_cMessage, "extracted_comments",
                    gettextpo_po_message_m_extracted_comments, 0);
  rb_define_method (rb_cMessage, "extracted_comments=",
                    gettextpo_po_message_m_extracted_comments_set, 1);
  rb_define_method (rb_cMessage, "prev_msgctxt",
                    gettextpo_po_message_m_prev_msgctxt, 0);
  rb_define_method (rb_cMessage,
                    "prev_msgctxt=", gettextpo_po_message_m_prev_msgctxt_set,
                    1);
  rb_define_method (rb_cMessage, "prev_msgid",
                    gettextpo_po_message_m_prev_msgid, 0);
  rb_define_method (rb_cMessage,
                    "prev_msgid=", gettextpo_po_message_m_prev_msgid_set, 1);
  rb_define_method (rb_cMessage, "prev_msgid_plural",
                    gettextpo_po_message_m_prev_msgid_plural, 0);
  rb_define_method (rb_cMessage, "prev_msgid_plural=",
                    gettextpo_po_message_m_prev_msgid_plural_set, 1);
  rb_define_method (rb_cMessage, "obsolete?", gettextpo_po_message_m_obsolete,
                    0);
  rb_define_method (rb_cMessage,
                    "obsolete=", gettextpo_po_message_m_obsolete_set, 1);
  rb_define_method (rb_cMessage, "fuzzy?", gettextpo_po_message_m_fuzzy, 0);
  rb_define_method (rb_cMessage, "fuzzy=", gettextpo_po_message_m_fuzzy_set,
                    1);

#ifdef HAVE_PO_MESSAGE_GET_FORMAT
  rb_define_method (rb_cMessage, "workflow_flag?",
                    gettextpo_po_message_m_workflow_flag_q, 1);
  rb_define_method (rb_cMessage, "update_workflow_flag",
                    gettextpo_po_message_m_update_workflow_flag, -1);
  rb_define_method (rb_cMessage, "workflow_flag_iterator",
                    gettextpo_po_message_m_workflow_flags_iterator, 0);
#endif

  rb_define_method (rb_cMessage, "format?", gettextpo_po_message_m_format_q,
                    1);
#ifdef HAVE_PO_MESSAGE_GET_FORMAT
  rb_define_method (rb_cMessage, "format", gettextpo_po_message_m_format, 1);
#endif
  rb_define_method (rb_cMessage, "update_format",
                    gettextpo_po_message_m_format_set, -1);

#ifdef HAVE_PO_MESSAGE_GET_FORMAT
  rb_define_method (rb_cMessage, "sticky_flag?",
                    gettextpo_po_message_m_sticky_flag_q, 1);
  rb_define_method (rb_cMessage, "update_sticky_flag",
                    gettextpo_po_message_m_update_sticky_flag, -1);
  rb_define_method (rb_cMessage, "sticky_flag_iterator",
                    gettextpo_po_message_m_sticky_flags_iterator, 0);
#endif

  rb_define_method (rb_cMessage, "range?", gettextpo_po_message_m_range, 1);
  rb_define_method (rb_cMessage, "range=", gettextpo_po_message_m_range_set,
                    1);
  rb_define_method (rb_cMessage, "filepos", gettextpo_po_message_m_filepos, 1);
  rb_define_method (rb_cMessage, "remove_filepos",
                    gettextpo_po_message_m_remove_filepos, 1);
  rb_define_method (rb_cMessage, "add_filepos",
                    gettextpo_po_message_m_add_filepos, 2);
  rb_define_method (rb_cMessage, "check_all", gettextpo_po_message_m_check_all,
                    -1);
  rb_define_method (rb_cMessage, "check_format",
                    gettextpo_po_message_m_check_format, -1);
  rb_cMessageIterator
      = rb_define_class_under (rb_mGettextPO, "MessageIterator", rb_cObject);
  rb_define_alloc_func (rb_cMessageIterator,
                        gettextpo_po_message_iterator_alloc);
  rb_define_method (rb_cMessageIterator, "next",
                    gettextpo_po_message_iterator_m_next, 0);
  rb_define_method (rb_cMessageIterator, "insert",
                    gettextpo_po_message_iterator_m_insert, 2);
  rb_cFilePos = rb_define_class_under (rb_mGettextPO, "FilePos", rb_cObject);
  rb_define_alloc_func (rb_cFilePos, gettextpo_po_filepos_alloc);
  rb_define_method (rb_cFilePos, "file", gettextpo_po_filepos_m_file, 0);
  rb_define_method (rb_cFilePos, "start_line",
                    gettextpo_po_filepos_m_start_line, 0);
#ifdef HAVE_PO_MESSAGE_GET_FORMAT
  rb_cFlagIterator
      = rb_define_class_under (rb_mGettextPO, "FlagIterator", rb_cObject);
  rb_define_alloc_func (rb_cFlagIterator, gettextpo_po_flag_iterator_alloc);
  rb_define_method (rb_cFlagIterator, "next",
                    gettextpo_po_flag_iterator_m_next, 0);
#endif
}
