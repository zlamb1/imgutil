#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext2.h"
#include "file.h"
#include "fs.h"

#define USE_ESCAPE_SEQUENCES

#ifdef USE_ESCAPE_SEQUENCES

#define ESC_RESET "\033[0m"
#define ESC_BOLD  "\033[1m"
#define ESC_RED   "\033[31m"

#else

#define ESC_RESET
#define ESC_BOLD
#define ESC_RED

#endif

static const char *cp_cmd_name = "ext2cp";

static file_t *img_file = NULL;
static int nsrc_files = 0;
static file_t **src_files = NULL;
static fs_t *fs = NULL;
static char *error_msg = NULL;

typedef struct
{
  const char *img;
  int nsrcs;
  const char **srcs;
  const char *dst;
} cp_params_t;

static void
cleanup (void)
{
  if (img_file != NULL)
    file_close (img_file);

  if (nsrc_files && src_files != NULL)
    for (int i = 0; i < nsrc_files; i++)
      {
        if (src_files[i] == NULL)
          continue;

        file_close (src_files[i]);
      }

  if (fs != NULL)
    ext2_fs_fini (fs);
}

static void
fail (const char *fmt, ...)
{
  const char *internal_err = "formatting error";
  char *buf = NULL;
  int tmp, _errno;
  va_list args;

  va_start (args, fmt);
  tmp = vasprintf (&buf, fmt, args);
  _errno = errno;

  cleanup ();

  if (tmp == -1)
    goto perror;

  fprintf (stderr, ESC_BOLD "%s: " ESC_RED "error: " ESC_RESET "%s\n",
           cp_cmd_name, buf);

  va_end (args);
  exit (1);

perror:
  if (buf != NULL)
    free (buf);

  if (_errno == -ENOMEM)
    internal_err = "out of memory";

  fprintf (stderr, ESC_BOLD "%s: " ESC_RED "error:" ESC_RESET "%s\n",
           cp_cmd_name, internal_err);

  exit (2);
}

static void
usage (void)
{
  printf ("Usage: %s [OPTION]... IMAGE SOURCE    DEST\n", cp_cmd_name);
  printf ("   or: %s [OPTION]... IMAGE SOURCE... DIRECTORY\n", cp_cmd_name);
}

static void
cp_op (cp_params_t *params)
{
  fs_init_error_t error;

  img_file = file_open (params->img, FILE_ORDWR);
  if (img_file == NULL)
    fail ("failed to open image file: '%s'", params->img);

  nsrc_files = params->nsrcs;
  src_files = malloc (sizeof (file_t *) * nsrc_files);
  if (src_files == NULL)
    fail ("out of memory");

  memset (src_files, 0, sizeof (file_t *) * nsrc_files);

  for (int i = 0; i < nsrc_files; i++)
    {
      src_files[i] = file_open (params->srcs[i], FILE_ORDONLY);
      if (src_files[i] == NULL)
        fail ("failed to open source file: '%s'", params->srcs[i]);
    }

  if (params->dst[0] != '/')
    fail ("destination must be absolute path");

  fs = ext2_fs_init (img_file, &error);
  if (fs == NULL)
    {
      if (error.allocated)
        error_msg = error.alloc_error;
      fail ("%s", error.const_error);
    }

  cleanup ();
}

int
main (int argc, const char **argv)
{
  cp_params_t params = { 0 };
  int argn, nsrcs = 0;

  /* pre-pass */
  for (argn = 1; argn < argc; argn++)
    {
      const char *arg = argv[argn];

      if (arg[0] == '-')
        {
          arg++;
          while (arg[0] != '\0')
            {
              switch (arg[0])
                {
                case 'h':
                  usage ();
                  exit (0);
                default:
                  fail ("invalid option '%c'", arg[1]);
                }
              arg++;
            }
        }

      if (params.img == NULL)
        params.img = arg;
      else
        ++nsrcs;
    }

  if (params.img == NULL)
    fail ("missing image operand");

  params.img = NULL;

  if (!nsrcs)
    fail ("missing source operand");

  if (!--nsrcs)
    fail ("missing destination operand");

  params.srcs = malloc (sizeof (const char *) * nsrcs);
  if (params.srcs == NULL)
    fail ("out of memory");

  for (argn = 1; argn < argc; argn++)
    {
      const char *arg = argv[argn];

      if (arg[0] == '-')
        continue;

      if (params.img == NULL)
        params.img = arg;
      else if (params.nsrcs < nsrcs)
        params.srcs[params.nsrcs++] = arg;
      else
        params.dst = arg;
    }

  assert (params.img);
  assert (params.nsrcs && params.srcs);
  assert (params.dst);

  cp_op (&params);

  return 0;
}