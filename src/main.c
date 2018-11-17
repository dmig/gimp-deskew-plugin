// GIMP deskew plug-in
//
// Copyright (C) 2007, 2008 Karl Chen <quarl@cs.berkeley.edu>

// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "config.h"

#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "interface.h"
#include "render.h"

#include "plugin-intl.h"


/*  Constants  */

#define PROCEDURE_NAME   "gimp_deskew_plugin"

#define DATA_KEY_VALS    "deskew"
#define DATA_KEY_UI_VALS "deskew_ui"

#define PARASITE_KEY     "deskew-options"


/*  Local function prototypes  */

static void   query (void);
static void   run   (const gchar      *name,
                     gint              nparams,
                     const GimpParam  *param,
                     gint             *nreturn_vals,
                     GimpParam       **return_vals);


/*  Local variables  */

const PlugInVals default_vals =
{
  0,
  1,
  2,
  0,
  FALSE
};

const PlugInImageVals default_image_vals =
{
  0
};

const PlugInDrawableVals default_drawable_vals =
{
  0
};

const PlugInUIVals default_ui_vals =
{
  TRUE
};

static PlugInVals         vals;
static PlugInImageVals    image_vals;
static PlugInDrawableVals drawable_vals;
static PlugInUIVals       ui_vals;
static int                args_num;
static GimpParamDef       args[] =
{
    { GIMP_PDB_INT32,    "run_mode",   "Interactive, non-interactive"    },
    { GIMP_PDB_IMAGE,    "image",      "Input image"                     },
    { GIMP_PDB_DRAWABLE, "drawable",   "Input drawable"                  },
    { GIMP_PDB_INT32,    "dummy",      "dummy1"                          },
    { GIMP_PDB_INT32,    "dummy",      "dummy2"                          },
    { GIMP_PDB_INT32,    "dummy",      "dummy3"                          },
    { GIMP_PDB_INT32,    "seed",       "Seed value (used only if randomize is FALSE)" },
    { GIMP_PDB_INT32,    "randomize",  "Use a random seed (TRUE, FALSE)" }
};

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
#if defined(G_OS_WIN32)
  gchar *gimp_share_directory;
#endif
  gchar *help_path;
  gchar *help_uri;

  args_num = G_N_ELEMENTS (args);

#if defined(G_OS_WIN32)
  gimp_share_directory = get_gimp_share_directory_on_windows();
#endif

#if defined(G_OS_WIN32)
  gimp_plugin_domain_register (GETTEXT_PACKAGE, gimp_locale_directory());
  help_path = g_build_filename (gimp_share_directory, PACKAGE_NAME, "help", NULL);
#else
  gimp_plugin_domain_register (GETTEXT_PACKAGE, LOCALEDIR);
  help_path = g_build_filename (DATADIR, "help", NULL);
#endif
  help_uri = g_filename_to_uri (help_path, NULL, NULL);
  g_free (help_path);

  gimp_plugin_help_register ("plugin-deskew-help", help_uri);
  g_free (help_uri);

  gimp_install_procedure (PROCEDURE_NAME,
                          N_("Deskew"),
                          N_("Auto-straighten (deskew) the image"),
                          "Dmitriy Geels <dmitriy.geels@gmail.com>",
                          "Karl Chen <quarl@cs.berkeley.edu>",
                          "2008, 2018",
                          N_("Deskew"),
                          "RGB*, GRAY*, INDEXED*",
                          GIMP_PLUGIN,
                          args_num, 0, args, NULL);

  gimp_plugin_menu_register (PROCEDURE_NAME, "<Image>/Layer/Transform");
}

static void
run (const gchar      *name,
     gint              n_params,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[1];
  GimpDrawable      *drawable;
  gint32             image_ID;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;

  /*  Initialize i18n support  */
#if defined(G_OS_WIN32)
  bindtextdomain (GETTEXT_PACKAGE, gimp_locale_directory());
#else
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#endif
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
  textdomain (GETTEXT_PACKAGE);

  args_num = G_N_ELEMENTS (args);

  run_mode = param[0].data.d_int32;
  image_ID = param[1].data.d_int32;
  drawable = gimp_drawable_get (param[2].data.d_drawable);

  /*  Initialize with default values  */
  vals          = default_vals;
  image_vals    = default_image_vals;
  drawable_vals = default_drawable_vals;
  ui_vals       = default_ui_vals;

  if (strcmp (name, PROCEDURE_NAME) == 0)
    {
      switch (run_mode)
        {
        case GIMP_RUN_NONINTERACTIVE:
          if (n_params != args_num)
            {
              fprintf(stderr, PLUGIN_NAME ": error: wrong number of arguments\n");
              fflush(stderr);
              status = GIMP_PDB_CALLING_ERROR;
            }
          else
            {
              vals.seed        = param[6].data.d_int32;
              vals.random_seed = param[7].data.d_int32;
            }
          break;

        case GIMP_RUN_INTERACTIVE:
          /*  Possibly retrieve data  */
          gimp_get_data (DATA_KEY_VALS,    &vals);
          gimp_get_data (DATA_KEY_UI_VALS, &ui_vals);

          // if (! dialog (image_ID, drawable,
          //               &vals, &image_vals, &drawable_vals, &ui_vals))
          //   {
          //     status = GIMP_PDB_CANCEL;
          //   }
          // break;

        case GIMP_RUN_WITH_LAST_VALS:
          /*  Possibly retrieve data  */
          gimp_get_data (DATA_KEY_VALS, &vals);

          break;

        default:
          break;
        }
    }
  else
    {
      status = GIMP_PDB_CALLING_ERROR;
    }

  if (status == GIMP_PDB_SUCCESS)
    {
      if (vals.random_seed)
        vals.seed = g_random_int ();

      render (image_ID, drawable, &vals, &image_vals, &drawable_vals);

      if (run_mode != GIMP_RUN_NONINTERACTIVE)
        gimp_displays_flush ();

      if (run_mode == GIMP_RUN_INTERACTIVE)
        {
          gimp_set_data (DATA_KEY_VALS,    &vals,    sizeof (vals));
          gimp_set_data (DATA_KEY_UI_VALS, &ui_vals, sizeof (ui_vals));
        }

      gimp_drawable_detach (drawable);
    }

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}


#if defined(G_OS_WIN32)
static gchar *
get_gimp_share_directory_on_windows()
{
  gchar ** tokens;
  gchar ** tokens2;
  gchar * str;
  gchar * ret;
  gint ind = 0;
  gint ind2;
  gboolean found = FALSE;

  tokens = g_strsplit(gimp_data_directory(), "\\", 1000);

  for (ind = 0; ind < 999; ++ind)
    {
      if (tokens[ind] == NULL)
        {
          break;
        }
      str = g_ascii_strdown(tokens[ind], -1);

      if (g_strcmp0(str, "share") == 0)
        {
          found = TRUE;
        }
      g_free(str);
      if (found)
        {
          break;
        }
    }

  if (!found)
    {
      g_message("GIMP share directory not found, resorting to default\n");
      ret = g_strdup_printf("C:\\Program Files\\GIMP-2.0\\share");
      return ret;
    }

  tokens2 = g_new(gchar*, ind + 2);
  for (ind2 = 0; ind2 <= ind; ++ind2)
    {
      tokens2[ind2] = g_strdup(tokens[ind2]);
    }
  tokens2[ind + 1] = NULL;
  g_strfreev(tokens);

  ret = g_strjoinv("\\", tokens2);

  g_strfreev(tokens2);
  if (!g_file_test(ret, G_FILE_TEST_IS_DIR))
    {
      g_message("GIMP share directory found but test for it failed, resorting to default\n");
      g_free(ret);
      ret = g_strdup_printf("C:\\Program Files\\GIMP-2.0\\share");
    }

  return ret;
}
#endif
