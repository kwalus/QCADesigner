#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vector_table.h"

#define DBG_VT(s)

typedef struct
  {
  VectorTable vt ;
  int icVRoom ;
  int icIRoom ;
  } VECTOR_TABLE_PRIVATE ;

static char *pszMagicCookie = "%%VECTOR TABLE%%" ;

int CountInputs (qcell *first_cell) ;
void FillInputs (VectorTable *pvt, qcell *first_cell) ;
void GetVTSizes (FILE *pfile, int *picInputs, int *picVectors) ;
gboolean CheckMagic (FILE *pfile) ;
char *ReadLine (FILE *pfile) ;
int ReadVector (FILE *pfile, gboolean *pVector, int ic) ;

VectorTable *VectorTable_new ()
  {
  VECTOR_TABLE_PRIVATE *pvt = malloc (sizeof (VECTOR_TABLE_PRIVATE)) ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_new\n")) ;
  
  pvt->vt.szFName[0] = 0 ;
  pvt->vt.num_of_vectors = 0 ;
  pvt->vt.vectors = NULL ;
  pvt->vt.num_of_inputs = 0 ;
  pvt->vt.inputs = NULL ;
  pvt->vt.active_flag = NULL ;
  pvt->icVRoom = 0 ;
  pvt->icIRoom = 0 ;
  
  DBG_VT (fprintf (stderr, "Exiting VectorTable_new\n")) ;

  return (VectorTable *)pvt ;
  }

VectorTable *VectorTable_copy (VectorTable *pvt)
  {
  int Nix ;
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  VECTOR_TABLE_PRIVATE *pvtpNew = (VECTOR_TABLE_PRIVATE *)VectorTable_new () ;
  
  g_snprintf (pvtpNew->vt.szFName, PATH_LENGTH, "%s", pvtp->vt.szFName) ;
  pvtpNew->vt.num_of_vectors = pvtpNew->icVRoom = pvtp->vt.num_of_vectors ;
  pvtpNew->vt.num_of_inputs = pvtpNew->icIRoom = pvtp->vt.num_of_inputs ;
  
  if (pvtp->vt.num_of_inputs > 0)
    {
    pvtpNew->vt.inputs = malloc (pvtp->vt.num_of_inputs * sizeof (qcell *)) ;
    memcpy (pvtpNew->vt.inputs, pvtp->vt.inputs, pvtp->vt.num_of_inputs * sizeof (qcell *)) ;
    pvtpNew->vt.active_flag = malloc (pvtp->vt.num_of_inputs * sizeof (gboolean)) ;
    memcpy (pvtpNew->vt.active_flag, pvtp->vt.active_flag, pvtp->vt.num_of_inputs * sizeof (gboolean)) ;
    }

  if (pvtp->vt.num_of_vectors > 0)
    {
    pvtpNew->vt.vectors = malloc (pvtp->vt.num_of_vectors * sizeof (gboolean *)) ;
    for (Nix = 0 ; Nix < pvtp->vt.num_of_vectors ; Nix++)
      {
      if (pvtp->vt.num_of_inputs > 0)
      	{
	pvtpNew->vt.vectors[Nix] = malloc (pvtp->vt.num_of_inputs * sizeof (gboolean)) ;
	memcpy (pvtpNew->vt.vectors[Nix], pvtp->vt.vectors[Nix], pvtp->vt.num_of_inputs * sizeof (gboolean)) ;
	}
      else
      	pvtpNew->vt.vectors[Nix] = NULL ;
      }
    }
  return (VectorTable *)pvtpNew ;
  }

VectorTable *VectorTable_clear (VectorTable *pvt)
  {
  int Nix ;
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;

  DBG_VT (fprintf (stderr, "Entering VectorTable_clear\n")) ;

  if (pvtp->icVRoom > 0)
    {
    if (pvtp->icIRoom > 0)
      for (Nix = 0 ; Nix < pvtp->icVRoom ; Nix++)
	free (pvtp->vt.vectors[Nix]) ;
    free (pvtp->vt.vectors) ;
    }

  if (pvtp->icIRoom > 0)
    {
    free (pvtp->vt.inputs) ;
    free (pvtp->vt.active_flag) ;
    }

  free (pvtp) ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_clear\n")) ;

  return NULL ;
  }

void VectorTable_fill (VectorTable *pvt, qcell *first_cell)
  {
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  int icInputs = CountInputs (first_cell) ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_fill\n")) ;

  if (icInputs > pvtp->icIRoom)
    {
    pvtp->icIRoom = icInputs ;
    pvtp->vt.inputs = realloc (pvtp->vt.inputs, icInputs * sizeof (qcell *)) ;
    pvtp->vt.active_flag = realloc (pvtp->vt.active_flag, icInputs * sizeof (gboolean)) ;
    }

  FillInputs ((VectorTable *)pvtp, first_cell) ;
  pvtp->vt.num_of_inputs = icInputs ;
  
  DBG_VT (fprintf (stderr, "Exiting VectorTable_fill.  pvt now looks like this:\n")) ;
  DBG_VT (VectorTable_dump (pvt, stderr)) ;

  pvtp->vt.num_of_vectors = 0 ;
  }

void VectorTable_add_input (VectorTable *pvt, qcell *new_input)
  {
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  int idx = pvtp->vt.num_of_inputs ;
  int Nix ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_add_input\n")) ;

  if (-1 == idx) return ;
  
  if (pvtp->vt.num_of_inputs == pvtp->icIRoom)
    {
    pvtp->icIRoom = pvtp->icIRoom * 2 + 1 ;
    pvtp->vt.inputs = realloc (pvtp->vt.inputs, pvtp->icIRoom * sizeof (qcell *)) ;
    pvtp->vt.active_flag = realloc (pvtp->vt.active_flag, pvtp->icIRoom * sizeof (gboolean)) ;
    
    for (Nix = 0 ; Nix < pvtp->icVRoom ; Nix++)
      pvtp->vt.vectors[Nix] = realloc (pvtp->vt.vectors[Nix], pvtp->icIRoom * sizeof (gboolean)) ;
    }
  
  if (idx < pvtp->vt.num_of_inputs)
    {
    memmove (&(pvtp->vt.inputs[idx + 1]), &(pvtp->vt.inputs[idx]), (pvtp->vt.num_of_inputs - idx) * sizeof (qcell *)) ;
    memmove (&(pvtp->vt.active_flag[idx + 1]), &(pvtp->vt.active_flag[idx]), (pvtp->vt.num_of_inputs - idx) * sizeof (gboolean)) ;
    for (Nix = 0 ; Nix < pvtp->vt.num_of_vectors ; Nix++)
      memmove (&(pvtp->vt.vectors[Nix][idx + 1]), &(pvtp->vt.vectors[Nix][idx]), (pvtp->vt.num_of_inputs - idx) * sizeof (gboolean)) ;
    }
  
  pvtp->vt.inputs[idx] = new_input ;
  pvtp->vt.active_flag[idx] = TRUE ;
  for (Nix = 0 ; Nix < pvtp->vt.num_of_vectors ; Nix++)
    pvtp->vt.vectors[Nix][idx] = FALSE ;
  
  pvtp->vt.num_of_inputs++ ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_add_input.  pvt now looks like this:\n")) ;
  DBG_VT (VectorTable_dump (pvt, stderr)) ;
  }

void VectorTable_del_input (VectorTable *pvt, qcell *old_input)
  {
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  int idx = -1 ;
  int Nix ;
  
  for (Nix = 0 ; Nix < pvtp->vt.num_of_inputs ; Nix++)
    if (pvtp->vt.inputs[Nix] == old_input)
      {
      idx = Nix ;
      break ;
      }
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_del_input\n")) ;

  if (-1 == idx) return ;
  
  if (idx < pvtp->vt.num_of_inputs - 1)
    {
    memmove (&(pvtp->vt.inputs[idx]), &(pvtp->vt.inputs[idx + 1]), (pvtp->vt.num_of_inputs - idx - 1) * sizeof (qcell *)) ;
    memmove (&(pvtp->vt.active_flag[idx]), &(pvtp->vt.active_flag[idx + 1]), (pvtp->vt.num_of_inputs - idx - 1) * sizeof (gboolean)) ;
    for (Nix = 0 ; Nix < pvtp->vt.num_of_vectors ; Nix++)
      memmove (&(pvtp->vt.vectors[Nix][idx]), &(pvtp->vt.vectors[Nix][idx + 1]), (pvtp->vt.num_of_inputs - idx - 1) * sizeof (gboolean)) ;
    }

  pvtp->vt.num_of_inputs-- ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_del_input.  pvt now looks like this:\n")) ;
  DBG_VT (VectorTable_dump (pvt, stderr)) ;
  }

int VectorTable_add_vector (VectorTable *pvt, int idxWanted)
  {
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  int idx = MAX (0, MIN (pvtp->vt.num_of_vectors, idxWanted)) ;
  int Nix ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_add_vector\n")) ;

  if (pvtp->vt.num_of_vectors == pvtp->icVRoom)
    {
    pvtp->icVRoom = pvtp->icVRoom * 2 + 1 ;
    pvtp->vt.vectors = realloc (pvtp->vt.vectors, pvtp->icVRoom * sizeof (gboolean *)) ;
    for (Nix = pvtp->vt.num_of_vectors ; Nix < pvtp->icVRoom ; Nix++)
      if (pvtp->icIRoom > 0)
      	pvtp->vt.vectors[Nix] = malloc (pvtp->icIRoom * sizeof (gboolean)) ;
      else
      	pvtp->vt.vectors[Nix] = NULL ;
    }
  
  if (idx < pvtp->vt.num_of_vectors)
    {
    gboolean *pVector = pvtp->vt.vectors[pvtp->vt.num_of_vectors] ;
    memmove (&(pvtp->vt.vectors[idx + 1]), &(pvtp->vt.vectors[idx]), (pvtp->vt.num_of_vectors - idx) * sizeof (gboolean *)) ;
    pvtp->vt.vectors[idx] = pVector ;
    }
  
  for (Nix = 0 ; Nix < pvtp->vt.num_of_inputs ; Nix++)
    pvtp->vt.vectors[idx][Nix] = FALSE ;
  
  pvtp->vt.num_of_vectors++ ;
  
  DBG_VT (fprintf (stderr, "Exiting VectorTable_add_vector\n")) ;

  return idx ;
  }

void VectorTable_del_vector (VectorTable *pvt, int idx)
  {
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_del_vector\n")) ;

  if (idx >= pvtp->vt.num_of_vectors) return ;
  
  if (idx < pvtp->vt.num_of_vectors - 1)
    {
    gboolean *pVector = pvtp->vt.vectors[idx] ;
    memmove (&(pvtp->vt.vectors[idx]), &(pvtp->vt.vectors[idx + 1]), (pvtp->vt.num_of_vectors - idx - 1) * sizeof (gboolean *)) ;
    pvtp->vt.vectors[pvtp->vt.num_of_vectors - 1] = pVector ;
    }
  
  DBG_VT (fprintf (stderr, "Exiting VectorTable_del_vector\n")) ;

  pvtp->vt.num_of_vectors-- ;
  }

gboolean VectorTable_save (VectorTable *pvt)
  {
  int Nix, Nix1 ;
  FILE *pfile = fopen (pvt->szFName, "w") ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_save\n")) ;

  if (NULL == pfile)
    {
    fprintf (stderr, "Unable to open vector table file.\n") ;
    return FALSE ;
    }

  fprintf (pfile, "%s\n", pszMagicCookie) ;
  
  fprintf (pfile,
    "# This is a vector table file.  All text beginning with a '#' and up to the\n"
    "# end of the line will be ignored.  The first vector is the list of active\n"
    "# inputs.  The inputs this vector table was constructed for are listed below\n"
    "# from Most Significant Bit to Least Significant Bit.\n") ;
  
  for (Nix = 0 ; Nix < pvt->num_of_inputs ; Nix++)
    fprintf (pfile, "# %s\n", pvt->inputs[Nix]->label) ;

  fprintf (pfile, "# The following vector is the active input mask.\n") ;
  
  for (Nix = 0 ; Nix < pvt->num_of_inputs ; Nix++)
    fprintf (pfile, "%d", pvt->active_flag[Nix] ? 1 : 0) ;
  fprintf (pfile, "\n") ;
  
  fprintf (pfile, "# Here are the vectors:\n") ;
  
  for (Nix = 0 ; Nix < pvt->num_of_vectors ; Nix++)
    {
    for (Nix1 = 0 ; Nix1 < pvt->num_of_inputs ; Nix1++)
      fprintf (pfile, "%d", pvt->vectors[Nix][Nix1] ? 1 : 0) ;
    fprintf (pfile, "\n") ;
    }
  
  fclose (pfile) ;

  DBG_VT (fprintf (stderr, "Exiting VectorTable_save\n")) ;

  return TRUE ;
  }

VTL_RESULT VectorTable_load (VectorTable *pvt)
  {
  int icInputs = 0, icVectors = 0, idxNext = -1, Nix, Nix1 ;
  VTL_RESULT ret = VTL_OK ;
  
  DBG_VT (fprintf (stderr, "Entering VectorTable_load\n")) ;

  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  FILE *pfile = fopen (pvt->szFName, "r") ;
  
  if (NULL == pfile) return VTL_FILE_FAILED ;
  
  if (!CheckMagic (pfile)) return VTL_MAGIC_FAILED ;
  
  DBG_VT (fprintf (stderr, "After CheckMagic the file is at %d\n", (int)ftell (pfile))) ;
  
  GetVTSizes (pfile, &icInputs, &icVectors) ;
  
  DBG_VT (fprintf (stderr, "There seem to be %d inputs and %d vectors in the file\n", icInputs, icVectors)) ;
  
  ret = icInputs < pvtp->vt.num_of_inputs ? VTL_SHORT :
      	icInputs > pvtp->vt.num_of_inputs ? VTL_TRUNC :
	VTL_OK ;
  
  DBG_VT (fprintf (stderr, "Before reading the active flags the file is at %d\n", (int)ftell (pfile))) ;

  if ((idxNext = ReadVector (pfile, pvtp->vt.active_flag, pvtp->vt.num_of_inputs)) < pvtp->vt.num_of_inputs)
    for (Nix = idxNext ; Nix < pvtp->vt.num_of_inputs ; Nix++)
      pvtp->vt.active_flag[Nix] = TRUE ;
  
  DBG_VT (fprintf (stderr, "Read the active flags.\n")) ;
  
  if (icVectors > pvtp->icVRoom)
    {
    pvtp->vt.vectors = realloc (pvtp->vt.vectors, icVectors * sizeof (gboolean *)) ;
    for (Nix = pvtp->icVRoom ; Nix < icVectors ; Nix++)
      if (pvtp->icIRoom > 0)
        pvtp->vt.vectors[Nix] = malloc (pvtp->icIRoom * sizeof (gboolean)) ;
      else
      	pvtp->vt.vectors[Nix] = NULL ;
    pvtp->icVRoom = icVectors ;
    }
  
  pvtp->vt.num_of_vectors = icVectors ;
  
  for (Nix = 0 ; Nix < icVectors ; Nix++)
    {
    DBG_VT (fprintf (stderr, "Reading vector %d\n", Nix)) ;
    if ((idxNext = ReadVector (pfile, pvtp->vt.vectors[Nix], pvtp->vt.num_of_inputs), pvtp->vt.num_of_inputs) < pvtp->vt.num_of_inputs)
      {
      DBG_VT (fprintf (stderr, "ReadVector has returned %d\n", idxNext)) ;
      for (Nix1 =  idxNext ; Nix1 < pvtp->vt.num_of_inputs ; Nix1++)
        pvtp->vt.vectors[Nix][Nix1] = FALSE ;
      }
    }
  
  fclose (pfile) ;
  
  DBG_VT (fprintf (stderr, "Exiting VectorTable_load.  pvt looks like this:\n")) ;
  DBG_VT (VectorTable_dump (pvt, stderr)) ;
  return ret ;
  }

void VectorTable_dump (VectorTable *pvt, FILE *pfile)
  {
  VECTOR_TABLE_PRIVATE *pvtp = (VECTOR_TABLE_PRIVATE *)pvt ;
  int Nix, Nix1 ;
  
  fprintf (pfile, "pvtp->vt.szFName = \"%s\"\n", pvtp->vt.szFName) ;
  fprintf (pfile, "pvtp->vt.num_of_vectors = %d\n", pvtp->vt.num_of_vectors) ;
  fprintf (pfile, "pvtp->vt.num_of_inputs = %d\n", pvtp->vt.num_of_inputs) ;
  fprintf (pfile, "pvtp->vt.inputs:\n") ;
  for (Nix = 0 ; Nix < pvtp->vt.num_of_inputs ; Nix++)
    fprintf (pfile, "pvtp->vt.inputs[%d]->label = \"%s\"\n", Nix, pvtp->vt.inputs[Nix]->label) ;
  fprintf (pfile, "pvtp->vt.active_flag:\n    ") ;
  for (Nix = 0 ; Nix < pvtp->vt.num_of_inputs ; Nix++)
    fprintf (pfile, "%5d ", Nix) ;
  fprintf (pfile, "\n    ") ;
  for (Nix = 0 ; Nix < pvtp->vt.num_of_inputs ; Nix++)
    fprintf (pfile, "%5s ", pvtp->vt.active_flag[Nix] ? "TRUE" : "FALSE") ;
  fprintf (pfile, "\npvtp->vt.vector:\n") ;
  for (Nix = 0 ; Nix < pvtp->vt.num_of_vectors ; Nix++)
    {
    fprintf (pfile, "%3d ", Nix) ;
    for (Nix1 = 0 ; Nix1 < pvtp->vt.num_of_inputs ; Nix1++)
      fprintf (pfile, "%5s ", pvtp->vt.vectors[Nix][Nix1] ? "TRUE" : "FALSE") ;
    fprintf (pfile, "\n") ;
    }
  fprintf (pfile, "pvtp->icIRoom = %d\n", pvtp->icIRoom) ;
  fprintf (pfile, "pvtp->icVRoom = %d\n", pvtp->icVRoom) ;
  }

int CountInputs (qcell *first_cell)
  {
  int ic = 0 ;

  DBG_VT (fprintf (stderr, "Entering CountInputs\n")) ;

  while (NULL != first_cell)
    {
    if (first_cell->is_input)
      {
      DBG_VT (fprintf (stderr, "Found cell with label \"%s\"\n", first_cell->label)) ;
      ic++ ;
      }
    first_cell = first_cell->next ;
    }

  DBG_VT (fprintf (stderr, "Exiting CountInputs\n")) ;

  return ic ;
  }

void FillInputs (VectorTable *pvt, qcell *first_cell)
  {
  int idx = 0 ;

  DBG_VT (fprintf (stderr, "Entering FillInputs\n")) ;

  while (NULL != first_cell)
    {
    if (first_cell->is_input)
      {
      pvt->inputs[idx] = first_cell ;
      pvt->active_flag[idx] = TRUE ;
      idx++ ;
      }
    first_cell = first_cell->next ;
    }

  DBG_VT (fprintf (stderr, "Exiting FillInputs\n")) ;

  }

gboolean CheckMagic (FILE *pfile)
  {
  int cb = ftell (pfile) ;
  char *psz = NULL ;
  gboolean bRet = FALSE ;
  
  fseek (pfile, 0, SEEK_SET) ;
  
  psz = ReadLine (pfile) ;
  
  bRet = !strcmp (psz, pszMagicCookie) ;
  
  free (psz) ;
  
  fseek (pfile, cb, SEEK_SET) ;
  
  return bRet ;
  }

char *ReadLine (FILE *pfile)
  {
  int idxBeg = ftell (pfile), idxEnd = idxBeg ;
  char c = 0, *pszRet = NULL ;
  int Nix, strl = 0 ;
  
  while ('\n' != c && !feof (pfile))
    fread (&c, 1, 1, pfile) ;
    
  idxEnd = ftell (pfile) ;
  
  if (idxEnd == idxBeg) return NULL ;
  
  pszRet = malloc ((strl = idxEnd - idxBeg) + 1) ;
  pszRet[idxEnd - idxBeg] = 0 ;
  fseek (pfile, idxBeg, SEEK_SET) ;
  fread (pszRet, idxEnd - idxBeg, 1, pfile) ;
  if ('\n' == pszRet[idxEnd - idxBeg - 1])
    pszRet[--strl] = 0 ;
  
  for (Nix = 0 ; Nix < strl ; Nix++)
   if ('#' == pszRet[Nix])
     pszRet[Nix] = 0 ;
  
  return pszRet ;
  }

void GetVTSizes (FILE *pfile, int *picInputs, int *picVectors)
  {
  int Nix ;
  int cb = ftell (pfile) ;
  char *psz = NULL ;
  
  DBG_VT (psz = ReadLine (pfile)) ;
  DBG_VT (fprintf (stderr, "File is at line '%s'\n", psz)) ;
  DBG_VT (fseek (pfile, cb, SEEK_SET)) ;
  DBG_VT (free (psz)) ;

  fseek (pfile, 0, SEEK_SET) ;
  
  *picInputs = *picVectors = 0 ;
  
  while (!feof (pfile))
    {
    if (NULL == (psz = ReadLine (pfile))) break ;
    
    for (Nix = 0 ; Nix < strlen (psz) ; Nix++)
      if ('0' == psz[Nix] || '1' == psz[Nix])
        (*picInputs)++ ;

    free (psz) ;
    if (0 != *picInputs)
      break ;
    }
  
  while (!feof (pfile))
    {
    if (NULL == (psz = ReadLine (pfile))) break ;
    
    for (Nix = 0 ; Nix < strlen (psz) ; Nix++)
      if ('0' == psz[Nix] || '1' == psz[Nix])
      	{
	(*picVectors)++ ;
	break ;
	}
    free (psz) ;
    }
  
  fseek (pfile, cb, SEEK_SET) ;
  }

int ReadVector (FILE *pfile, gboolean *pVector, int ic)
  {
  int idx = -1 ;
  int Nix ;
  char *psz ;
  DBG_VT (int cb = ftell (pfile)) ;
  DBG_VT (psz = ReadLine (pfile)) ;
  DBG_VT (fprintf (stderr, "File is at line '%s'\n", psz)) ;
  DBG_VT (fseek (pfile, cb, SEEK_SET)) ;
  DBG_VT (free (psz)) ;

  while (!feof (pfile))
    {
    DBG_VT (fprintf (stderr, "Entering non-EOF loop\n")) ;
    if (NULL == (psz = ReadLine (pfile))) return 0 ;
    DBG_VT (fprintf (stderr, "ReadVector: Have retrieved the following line:\n%s\n", psz)) ;
    
    for (Nix = 0 ; Nix < strlen (psz) && idx < ic - 1 ; Nix++)
      {
      if ('0' == psz[Nix] || '1' == psz[Nix])
        pVector[++idx] = ('1' == psz[Nix]) ;
      }
    
    free (psz) ;
    
    DBG_VT () ;
    if (-1 != idx) break ;
    }
  DBG_VT (fprintf (stderr, "ReadVector returning idx = %d\n", idx)) ;
  return idx ;
  }
