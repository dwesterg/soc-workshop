/*
 * Copyright (c) 2013, Altera Corporation.
 * All rights reserved.
 * 
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD 3-Clause license below:
 * 
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 * 
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 * 
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 * 
 *      - Neither Altera nor the names of its contributors may be 
 *         used to endorse or promote products derived from this 
 *         software without specific prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

/* Reprogram the board with given SOF. */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "pcie_linux_driver_exports.h"
#include "pkg_editor/pkg_editor.h"


/* Local include */
#include "version.h"


#define DEBUG 0

/* Returns 1 if file is readable, 0 if it's not. */
int file_readable (const char *filename) {
  FILE *f = fopen (filename, "rb");
  if (f == NULL) {
    fprintf (stderr, "Couldn't open file %s for reading\n", filename);
    return 0;
  } else {
    fclose(f);
    return 1;
  }
}

/* Disable communication bridges between ARM and FPGA */
void disable_bridges() {
  system ("echo 0 > /sys/class/fpga-bridge/fpga2hps/enable");
  system ("echo 0 > /sys/class/fpga-bridge/hps2fpga/enable");
  system ("echo 0 > /sys/class/fpga-bridge/lwhps2fpga/enable");
}

/* Enable communication bridges between ARM and FPGA */
void enable_bridges() {
  system ("echo 1 > /sys/class/fpga-bridge/fpga2hps/enable");
  system ("echo 1 > /sys/class/fpga-bridge/hps2fpga/enable");
  system ("echo 1 > /sys/class/fpga-bridge/lwhps2fpga/enable");
}

/* Return 1 if FPGA is in user mode. */  
int fpga_in_user_mode() {
  #define BUF_SIZE 1024
  char buf[BUF_SIZE];
  const char *status_file = "/sys/class/fpga/fpga0/status";
  char *fgets_res = NULL;  
  
  FILE *status = fopen (status_file, "r");
  if (status == NULL) {
    fprintf (stderr, "Couldn't open FPGA status from %s!\n", status_file);
    return 0;
  }
  
  fgets_res = fgets (buf, BUF_SIZE, status);
  fclose (status);
    
  if (fgets_res == NULL) {
    fprintf (stderr, "Couldn't read FPGA status from %s!\n", status_file);
    return 0;
  }
    
  if (strstr (buf, "user mode") == NULL) {
    fprintf (stderr, "After reprogramming, FPGA is not in user mode (%s)!\n", buf);
    return 0;
  } else {
    printf ("Reprogramming was successful!\n");
  }
  
  /* If here, FPGA is in user mode */
  return 1;
}

/* Disabled interrupts on the FPGA */
void prepare_fpga_for_reprogramming (ssize_t f) {

  struct acl_cmd read_cmd = { 
    ACLPCI_CMD_BAR, 
    ACLPCI_CMD_SAVE_PCI_CONTROL_REGS, 
    NULL,
    NULL, 
    0 };
  
  read (f, &read_cmd, sizeof(read_cmd));
  disable_bridges();
}

void enable_fpga_after_reprogramming (ssize_t f) {

  struct acl_cmd read_cmd = { 
    ACLPCI_CMD_BAR, 
    ACLPCI_CMD_LOAD_PCI_CONTROL_REGS, 
    NULL,
    NULL, 
    0 };
  
  enable_bridges();
  read (f, &read_cmd, sizeof(read_cmd));
}


char *acl_loadFileIntoMemory (const char *in_file, size_t *file_size_out) {

  FILE *f = NULL;
  char *buf;
  size_t file_size;
  
  // When reading as binary file, no new-line translation is done.
  f = fopen (in_file, "rb");
  if (f == NULL) {
    fprintf (stderr, "Couldn't open file %s for reading\n", in_file);
    return NULL;
  }
  
  // get file size
  fseek (f, 0, SEEK_END);
  file_size = ftell (f);
  rewind (f);
  
  // slurp the whole file into allocated buf
  buf = (char*) malloc (sizeof(char) * file_size);
  *file_size_out = fread (buf, sizeof(char), file_size, f);
  fclose (f);
  
  if (*file_size_out != file_size) {
    fprintf (stderr, "Error reading %s. Read only %zu out of %zu bytes\n", 
                     in_file, *file_size_out, file_size);
    return NULL;
  }
  return buf;
}

/* return 1 if given filename has given extension */
int filename_has_ext (const char *filename, const char *ext)
{
   size_t ext_len = strlen (ext);  
   size_t fn_len  = strlen (filename);
   if (ext_len > fn_len) {
      return 0;
   } else {
      return (strcmp (filename + strlen(filename) - ext_len, ext) == 0);
   }
}

/* Given 'filename', make aocx point to it if filename ends with ".aocx"
 * or make rbf point to it if filename ends with ".rbf". */
void map_filenames_to_aocx_and_rbf (const char* filename,
          const char** aocx, const char **rbf, const char **fpga_bin) {

  assert (aocx != NULL && rbf != NULL && fpga_bin != NULL);
  assert (filename != NULL);
  
  *aocx = NULL;
  *rbf = NULL;
  *fpga_bin = NULL;
  if (filename_has_ext (filename, ".rbf") || 
      filename_has_ext (filename, ".RBF") ) {
    *rbf = filename;
  }
  if (filename_has_ext (filename, ".aocx") || 
      filename_has_ext (filename, ".AOCX") ) {
    *aocx = filename;
  }
  if (filename_has_ext (filename, ".bin") || 
      filename_has_ext (filename, ".BIN") ) {
    *fpga_bin = filename;
  }

  return;
}

/* Reprogram FPGA with RBF file */
int reprogram_with_rbf (ssize_t f, const char *rbf) {

  #define BUF_SIZE 1024
  char buf[BUF_SIZE];
  
  if (rbf == NULL || !file_readable(rbf)) {
    return 1;
  }
  
  /* Send the programming file to the FPGA Manager */
  sprintf (buf, "cat %s > /dev/fpga0", rbf);
  prepare_fpga_for_reprogramming(f);
  system (buf);
  enable_fpga_after_reprogramming(f);
  
  if (fpga_in_user_mode()) {
    return 0;
  } else {
    return 1;
  }
}


/* Reprogram with fpga.bin package section in memory */
int reprogram_with_fpga_bin_image (ssize_t f, struct acl_pkg_file *fpga_bin_pkg) {

  /* Find .acl.rbf section from within the .fpga.bin section */
  char tmp_filename[FILENAME_MAX];
  size_t rbf_size;
  int res;
  
  if (!acl_pkg_section_exists (fpga_bin_pkg, ACL_PKG_SECTION_RBF, &rbf_size)) {
    printf ("FPGA programming section of file does not contain an RBF.\n");
    acl_pkg_list_file_sections (fpga_bin_pkg);
    return 1;
  }
  
  tmpnam(tmp_filename);
  if (!acl_pkg_read_section_into_file (fpga_bin_pkg, ACL_PKG_SECTION_RBF, tmp_filename)) {
    printf ("Cannot read RBF from the FPGA programming section into tmp file %s.\n", tmp_filename);
    return 1;
  }
  
  /* Now rbf_sect points to beginning of the RBF section and rbf_size contains
   * size of that section. So can call FPGA Manager on this directly. */
  res = reprogram_with_rbf (f, tmp_filename); 
  
  if (remove (tmp_filename) != 0) {
    printf ("Couldn't delete temporary RBF file %s\n", tmp_filename);
    /* non-fatal error */
  }
  
  return res;
}


/* Reprogram with fpga.bin file */
int reprogram_with_fpga_bin (ssize_t f, const char *fpga_bin) {

  size_t fpga_bin_size = 0;
  char *fpga_bin_image = acl_loadFileIntoMemory (fpga_bin, &fpga_bin_size);
  int ret = 0;
  
  if (fpga_bin_image == NULL) {
    return 1;
  }
  struct acl_pkg_file *pkg = acl_pkg_open_file_from_memory (fpga_bin_image, fpga_bin_size, ACL_PKG_SHOW_ERROR);
  if (pkg == NULL) {
    printf ("Cannot read given fpga.bin file %s!\n", fpga_bin);
    return 1;
  }

  ret = reprogram_with_fpga_bin_image(f, pkg);
  free (fpga_bin_image);
  return ret;
}


/* Reprogram FPGA with AOCX file.
 * Extract RBF from AOCX and then do reprorgram with that image. */
int reprogram_with_aocx (ssize_t f, const char *aocx) {

  size_t aocx_size = 0;
  char *aocx_image = acl_loadFileIntoMemory (aocx, &aocx_size);
  if (aocx_image == NULL) {
    goto bad_exit;
  }
  
  struct acl_pkg_file *pkg = acl_pkg_open_file_from_memory (aocx_image, aocx_size, ACL_PKG_SHOW_ERROR | ACL_PKG_SHOW_INFO);
  if (pkg == NULL) {
    printf ("Cannot read given aocx file %s!\n", aocx);
    goto bad_exit;
  }
  
  /* Get .fpga.bin section, which contains .rbf */
  char *fpga_bin_sect = NULL;
  size_t fpga_bin_size;
  if (!acl_pkg_section_exists (pkg, ACL_PKG_SECTION_FPGA_BIN, &fpga_bin_size)) {
    printf ("aocx file %s does not contain FPGA programming section.\n", aocx);
    goto bad_exit;
  }
  if (!acl_pkg_read_section_transient (pkg, ACL_PKG_SECTION_FPGA_BIN, &fpga_bin_sect)) {
    printf ("Cannot read FPGA programming section from aocx file %s.\n", aocx);
    goto bad_exit;
  }
  
  /* Now open .fpga.bin section (already in memory) as another pkg */
  struct acl_pkg_file *fpga_bin_pkg = NULL;
  fpga_bin_pkg = acl_pkg_open_file_from_memory (fpga_bin_sect, fpga_bin_size, ACL_PKG_SHOW_ERROR | ACL_PKG_SHOW_INFO);
  if (fpga_bin_pkg == NULL) {
    printf ("Cannot read FPGA programming section from file %s.\n", aocx);
    goto bad_exit;
  }
  
  return reprogram_with_fpga_bin_image (f, fpga_bin_pkg);
  
bad_exit:
  free(aocx_image);
  return 1;
}


int main(int argc, char **argv) {
  
  /* Ignore kernel-completion signal.
   * If a host program was killed right after launching a kernel,
   * the device might keep sending kernel-done signals. That would kill
   * this exe unless we specifically ignore these signals */
  struct sigaction sig;
  sig.sa_sigaction = NULL;
  sig.sa_handler = SIG_IGN;
  sig.sa_flags = SIGEV_NONE;
  sigaction(SIG_INT_NOTIFY, &sig, NULL);
  int ret;
  
  
  if (argc != 3) {
    printf ("Usage: reprogram device [RBF|AOCX]\n");
    return 1;
  }

  /* Open the device */
  ssize_t f = open (argv[1], O_RDWR);
  if (f == -1) {
    printf ("Couldn't open %s device! Did you load the driver?\n", argv[1]);
    return 1;
  }
  
  const char *rbf = NULL, *aocx = NULL, *fpga_bin = NULL;
  const char *filename = argv[2];
  map_filenames_to_aocx_and_rbf (filename, &aocx, &rbf, &fpga_bin);
  
  if (rbf != NULL) {
    ret = reprogram_with_rbf (f, rbf);
  } else if (aocx != NULL) {
    ret = reprogram_with_aocx (f, aocx);
  } else if (fpga_bin != NULL) {
    ret = reprogram_with_fpga_bin (f, fpga_bin);
  } else {
    printf ("Given file %s is neither an RBF nor an AOCX file.\n", filename);
  }

  close (f);
  return ret;
}

