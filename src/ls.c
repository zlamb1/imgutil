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

static const char *cp_cmd_name = "ext2ls";

static file_t *img_file = NULL;
static int nfiles = 0;
static file_t **files = NULL;
static fs_t *fs = NULL;
static char *error_msg = NULL;

typedef struct
{
  const char *img;
  int nfiles;
  const char **files;
} ls_params_t;

static void
cleanup (void)
{
  if (img_file != NULL)
    file_close (img_file);

  if (nfiles && files != NULL)
    for (int i = 0; i < nfiles; i++)
      {
        if (files[i] == NULL)
          continue;

        file_close (files[i]);
      }

  if (fs != NULL)
    ext2_fs_fini (fs);

  if (error_msg != NULL)
    free (error_msg);
}

static void
fail (const char *fmt, ...)
{
  const char *internal_err = "printf error";
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
  printf ("Usage: %s [OPTION]... IMAGE FILE...\n", cp_cmd_name);
}

static void
ls_op (ls_params_t *params)
{
  fs_init_error_t error;

  img_file = file_open (params->img, FILE_ORDWR);
  if (img_file == NULL)
    fail ("failed to open image file: '%s'", params->img);

  nfiles = params->nfiles;
  files = malloc (sizeof (file_t *) * nfiles);
  if (files == NULL)
    fail ("out of memory");

  memset (files, 0, sizeof (file_t *) * nfiles);

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
  ls_params_t params = { 0 };
  int argn, _nfiles = 0;

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
        ++_nfiles;
    }

  if (params.img == NULL)
    fail ("missing image operand");

  params.img = NULL;

  if (!_nfiles)
    fail ("missing source operand");

  if (!--_nfiles)
    fail ("missing destination operand");

  params.files = malloc (sizeof (const char *) * _nfiles);
  if (params.files == NULL)
    fail ("out of memory");

  params.img = NULL;

  for (int i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-')
        continue;

      if (params.img == NULL)
        params.img = argv[i];
      else
        params.files[params.nfiles++] = argv[i];
    }

  assert (params.img);
  assert (params.nfiles && params.files);

  ls_op (&params);

  return 0;
}