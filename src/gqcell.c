#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <stdlib.h>
#include <math.h>
#include <glib-object.h>


#include "gqcell.h"
#include "fileio_helpers.h"

#define DBG_GQC(s)
#define DBG_GQC_READ(s)

static GdkColor clrClock[4] =
  {{0, 0x0000, 0xFFFF, 0x0000},
   {0, 0xF000, 0x0FFF, 0xF000},
   {0, 0x0000, 0xFFFF, 0xFFFF},
   {0, 0xFFF0, 0xFFFF, 0xFFF0}} ;
static GdkColor clrOrange = {0, 0xFFFF, 0x8000, 0x0000} ;
static GdkColor clrYellow = {0, 0xFFFF, 0xFFFF, 0x0000} ;
static GdkColor clrRed    = {0, 0xFFFF, 0x0000, 0x0000} ;
static GdkColor clrBlue   = {0, 0x0000, 0x0000, 0xFFFF} ;

static void gqcell_class_init (GObjectClass *klass, gpointer data) ;
// static void gqcell_class_finalize (GObjectClass *klass, gpointer data) ;
static void gqcell_instance_init (GObject *object, gpointer data) ;
static void gqcell_instance_finalize (GObject *object) ;
static void FindNameAndValue (char *psz, char **ppszName, char **ppszValue) ;
// needed by gqcell_new_from_stream
static inline GdkColor *colour_from_int (int clr) ;
static inline int int_from_colour (GdkColor *clr) ;

GType gqcell_get_type ()
  {
  static GType gqcell_type = 0 ;
  
  if (!gqcell_type)
    {
    static const GTypeInfo gqcell_info = 
      {
      sizeof (GQCellClass),
      (GBaseInitFunc)NULL, 
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)gqcell_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof(GQCell),
      0,
      (GInstanceInitFunc)gqcell_instance_init
      } ;
    gqcell_type = g_type_register_static (G_TYPE_OBJECT, "GQCell", &gqcell_info, 0) ;
    }
  return gqcell_type ;
  }

static void gqcell_class_init (GObjectClass *klass, gpointer data)
  {
  GdkColormap *clrmSys = gdk_colormap_get_system () ;
  gdk_color_alloc (clrmSys, &clrClock[0]) ;
  gdk_color_alloc (clrmSys, &clrClock[1]) ;
  gdk_color_alloc (clrmSys, &clrClock[2]) ;
  gdk_color_alloc (clrmSys, &clrClock[3]) ;
  gdk_color_alloc (clrmSys, &clrOrange) ;
  gdk_color_alloc (clrmSys, &clrYellow) ;
  gdk_color_alloc (clrmSys, &clrRed) ;
  gdk_color_alloc (clrmSys, &clrBlue) ;

  G_OBJECT_CLASS (klass)->finalize = gqcell_instance_finalize ;
  }

static void gqcell_instance_init (GObject *object, gpointer data)
  {
  }

static void gqcell_instance_finalize (GObject *object)
  {
  GObjectClass *parent_class ;
  
  DBG_GQC (fprintf (stderr, "Killing instance of GQCell object.\n")) ;
  
  parent_class = g_type_class_peek_parent (G_OBJECT_GET_CLASS (object)) ;
  parent_class->finalize (object) ;
  
  g_free (GQCELL (object)->label) ;
  g_free (GQCELL (object)->cell_dots) ;
  }

//////////////////////////////////
// Useful code starts here      //
//////////////////////////////////

GQCell *gqcell_new (GQCell *tmplt, double x, double y, cell_OP *pco, int type)
  {
  GQCell *gqcRet = NULL ;
  
  if (NULL != tmplt && TYPE_USECELL == type)
    gqcRet = gqcell_copy (tmplt) ;

  else if (TYPE_USECELL != type)
    {
    double dcx = 0, dcy = 0, ddiam = 0 ;
    
    if (TYPE_2 == type)
      {
      dcx = pco->type_2_cell_width ;
      dcy = pco->type_2_cell_height ;
      ddiam = pco->type_2_dot_diameter ;
      }
    else /* create TYPE_1 cells by default */
      {
      dcx = pco->type_1_cell_width ;
      dcy = pco->type_1_cell_height ;
      ddiam = pco->type_1_dot_diameter ;
      }
    
    gqcRet = g_object_new (TYPE_GQCELL, NULL) ;
    
    gqcRet->cell_model = NULL ;
    
    gqcRet->x = x ;
    gqcRet->y = y ;
    gqcRet->cell_width = dcx ;
    gqcRet->cell_height = dcy ;
    
    gqcRet->top_x = gqcRet->x - gqcRet->cell_width / 2;
    gqcRet->top_y = gqcRet->y - gqcRet->cell_height / 2;
    gqcRet->bot_x = gqcRet->x + gqcRet->cell_width / 2;
    gqcRet->bot_y = gqcRet->y + gqcRet->cell_height / 2;
    
    gqcRet->is_input = FALSE ;
    gqcRet->is_output = FALSE ;
    gqcRet->is_fixed = FALSE ;
    
    gqcRet->number_of_dots = 4 ;
    gqcRet->cell_dots = g_malloc (gqcRet->number_of_dots * sizeof (qdot)) ;
    
    gqcRet->cell_dots[0].charge = HALF_QCHARGE ;
    gqcRet->cell_dots[1].charge = HALF_QCHARGE ;
    gqcRet->cell_dots[2].charge = HALF_QCHARGE ;
    gqcRet->cell_dots[3].charge = HALF_QCHARGE ;
    
    gqcRet->cell_dots[0].diameter = ddiam ;
    gqcRet->cell_dots[1].diameter = ddiam ;
    gqcRet->cell_dots[2].diameter = ddiam ;
    gqcRet->cell_dots[3].diameter = ddiam ;

    gqcRet->cell_dots[3].x = x - (dcx / 2 - ddiam) / 2 - ddiam / 2;
    gqcRet->cell_dots[3].y = y - (dcx / 2 - ddiam) / 2 - ddiam / 2;
    gqcRet->cell_dots[0].x = x + (dcx / 2 - ddiam) / 2 + ddiam / 2;
    gqcRet->cell_dots[0].y = y - (dcx / 2 - ddiam) / 2 - ddiam / 2;
    gqcRet->cell_dots[2].x = x - (dcx / 2 - ddiam) / 2 - ddiam / 2;
    gqcRet->cell_dots[2].y = y + (dcx / 2 - ddiam) / 2 + ddiam / 2;
    gqcRet->cell_dots[1].x = x + (dcx / 2 - ddiam) / 2 + ddiam / 2;
    gqcRet->cell_dots[1].y = y + (dcx / 2 - ddiam) / 2 + ddiam / 2;

    gqcRet->label = g_strdup ("NO NAME") ;
    gqcRet->clock = pco->default_clock ;
    
    gqcell_reset_colour (gqcRet) ;
    }
  
  return gqcRet ;
  }
  
void gqcell_scale(GQCell *cell, double scale){

	cell->x *= scale;
	cell->y *= scale;
	cell->top_x *= scale;
	cell->top_y *= scale;
	cell->bot_x *= scale;
	cell->bot_y *= scale;
	
	cell->cell_height *= scale;
	cell->cell_width *= scale;
	
	cell->cell_dots[0].diameter *= scale;
    cell->cell_dots[1].diameter *= scale;
    cell->cell_dots[2].diameter *= scale;
    cell->cell_dots[3].diameter *= scale;

    cell->cell_dots[3].x *= scale;
    cell->cell_dots[3].y *= scale;
    cell->cell_dots[0].x *= scale;
    cell->cell_dots[0].y *= scale;
    cell->cell_dots[2].x *= scale;
    cell->cell_dots[2].y *= scale;
    cell->cell_dots[1].x *= scale;
    cell->cell_dots[1].y *= scale;
}  

GQCell *gqcell_copy (GQCell *tmplt)
  {
  GQCell *gqcRet = g_object_new (TYPE_GQCELL, NULL) ;
  
  gqcRet->cell_model = tmplt->cell_model ;
  gqcRet->x = tmplt->x ;
  gqcRet->y = tmplt->y ;
  gqcRet->top_x = tmplt->top_x ;
  gqcRet->top_y = tmplt->top_y ;
  gqcRet->bot_x = tmplt->bot_x ;
  gqcRet->bot_y = tmplt->bot_y ;
  gqcRet->cell_width = tmplt->cell_width ;
  gqcRet->cell_height = tmplt->cell_height ;
  gqcRet->orientation = tmplt->orientation ;
  gqcRet->number_of_dots = tmplt->number_of_dots ;
  gqcRet->color = tmplt->color ;
  gqcRet->clock = tmplt->clock ;
  gqcRet->is_input = tmplt->is_input ;
  gqcRet->is_output = tmplt->is_output ;
  gqcRet->is_fixed = tmplt->is_fixed ;
  gqcRet->label = g_strdup (tmplt->label) ;
  gqcRet->cell_dots = g_malloc (gqcRet->number_of_dots * sizeof (qdot)) ;
  memcpy (gqcRet->cell_dots, tmplt->cell_dots, gqcRet->number_of_dots * sizeof (qdot)) ;
  
  return gqcRet ;
  }

void gqcell_link (GQCell *gqc, GQCell *prev, GQCell *next)
  {
  if (NULL != prev)
    {
    gqc->next = prev->next ;
    gqc->prev = prev ;
    prev->next = gqc ;
    if (NULL != gqc->next)
      gqc->next->prev = gqc ;
    }
  else if (NULL != next)
    {
    gqc->next = next ;
    gqc->prev = prev ;
    gqc->next->prev = gqc ;
    if (NULL != gqc->prev)
      gqc->prev->next = gqc ;
    }
  else /* both prev and next are NULL */
    gqc->prev = gqc->next = NULL ;
  
  g_object_ref (gqc) ;
  }

void gqcell_unlink (GQCell *gqc)
  {
  if (NULL != gqc->prev)
    gqc->prev->next = gqc->next ;
  if (NULL != gqc->next)
    gqc->next->prev = gqc->prev ;
  
  gqc->prev = gqc->next = NULL ;
  g_object_unref (gqc) ;
  }

void gqcell_select (GQCell *gqc)
  {
  gqc->color = &clrRed ;
  }

void gqcell_reset_colour (GQCell *gqc)
  {
  gqc->color =
    gqc->is_input   ? &clrBlue :
    gqc->is_output  ? &clrYellow :
    gqc->is_fixed   ? &clrOrange : &(clrClock[gqc->clock]) ;
  }

// rotates the qdots within a cell by a given angle //
//-------------------------------------------------------------------//
void gqcell_rotate (GQCell * cell, float angle)
{

    int i;
    double x;
    double y;

    g_assert(cell != NULL);

    // -- uses standard rotational transform -- //
    for (i = 0; i < cell->number_of_dots; i++) {

	g_assert((cell->cell_dots + i) != NULL);

	x =
	    cell->x + (cell->cell_dots[i].x -
		       cell->x) * (float) cos(angle) -
	    (cell->cell_dots[i].y - cell->y) * (float) sin(angle);
	y =
	    cell->y + (cell->cell_dots[i].y -
		       cell->y) * (float) cos(angle) +
	    (cell->cell_dots[i].x - cell->x) * (float) sin(angle);
	cell->cell_dots[i].x = x;
	cell->cell_dots[i].y = y;

    }

}				//rotate_cell

// -- If a cell is made an input then this is used to set its input name -- //
void gqcell_set_label (GQCell * cell, char *label)
{

    g_assert(cell != NULL);
    g_assert(label != NULL);

    // free up the old name //
    g_free(cell->label);
    
    cell->label = g_strdup (label) ;
    
    DBG_GQC (fprintf (stderr, "GQCell:My label is now %s\n", cell->label)) ;

}				//set_cell_label


void gqcell_move_to_location (GQCell *cell, float x, float y)
{

    int i = 0;

    g_assert(cell != NULL);

    for (i = 0; i < cell->number_of_dots; i++) {
	cell->cell_dots[i].x += x - cell->x;
	cell->cell_dots[i].y += y - cell->y;
    }

    cell->x = x;
    cell->y = y;

    cell->top_x = x - cell->cell_width / 2;
    cell->top_y = y - cell->cell_height / 2;

    cell->bot_x = x + cell->cell_width / 2;
    cell->bot_y = y + cell->cell_height / 2;
} //gqcell_move_to_location

// -- moves a cell by a given offset -- //
void gqcell_move_by_offset (GQCell *cell, float x_offset, float y_offset)
{

  int i;

  g_assert(cell != NULL);

  for (i = 0; i < cell->number_of_dots; i++) {
      cell->cell_dots[i].x += x_offset;
      cell->cell_dots[i].y += y_offset;
  }

  cell->x += x_offset;
  cell->y += y_offset;

  cell->top_x = cell->x - cell->cell_width / 2;
  cell->top_y = cell->y - cell->cell_height / 2;

  cell->bot_x = cell->x + cell->cell_width / 2;
  cell->bot_y = cell->y + cell->cell_height / 2;


} //move_cell_by_offset

void gqcell_set_as_input (GQCell *cell)
{
  g_assert(cell != NULL);

  cell->is_output = FALSE;
  cell->is_fixed = FALSE;

  cell->is_input = TRUE;
  cell->color = &clrBlue;
} //set_cell_as_input

void gqcell_set_as_normal (GQCell *cell)
{
  g_assert(cell != NULL);

  cell->is_output = FALSE;
  cell->is_fixed = FALSE;

  cell->is_input = FALSE;
  cell->color = &clrClock[cell->clock];
} //set_cell_as_input

// sets a cell as an output cell and gives it a unique output number //
void gqcell_set_as_output (GQCell *cell)
{
  g_assert(cell != NULL);

  cell->is_input = FALSE;
  cell->is_fixed = FALSE;

  cell->is_output = TRUE;
  cell->color = &clrYellow;
} //set_cell_as_output

void gqcell_set_as_fixed (GQCell *cell)
{
  g_assert(cell != NULL);

  cell->is_input = FALSE;
  cell->is_output = FALSE;

  cell->is_fixed = TRUE;
  cell->color = &clrOrange;
} //set_cell_as_fixed

// calculates the polarization of a qcell //
double gqcell_calculate_polarization (GQCell *cell){

  g_assert (cell != NULL);
  g_assert (cell->number_of_dots == 4);

  return ((cell->cell_dots[0].charge + cell->cell_dots[2].charge) -
	  (cell->cell_dots[1].charge + cell->cell_dots[3].charge)) / (4 * HALF_QCHARGE);
} //calculate_polarization

//-------------------------------------------------------------------//

//!Sets the polarization of the given cell by setting the appropriate charges to each of the quantum-dots.
//!At this time only works with cells that have 4 QD's.
void gqcell_set_polarization (GQCell *cell, double new_polarization){

  g_assert (cell != NULL);
  g_assert (cell->number_of_dots == 4);

  cell->cell_dots[0].charge = HALF_QCHARGE * new_polarization + HALF_QCHARGE;
  cell->cell_dots[2].charge = HALF_QCHARGE * new_polarization + HALF_QCHARGE;
  cell->cell_dots[1].charge = -1 * HALF_QCHARGE * new_polarization + HALF_QCHARGE;
  cell->cell_dots[3].charge = -1 * HALF_QCHARGE * new_polarization + HALF_QCHARGE;

}//set_cell_polarization

// write the cell to a stream
void gqcell_serialize (GQCell *cell, FILE *stream)
  {
  int i ;
  fprintf(stream, "[QCELL]\n");

  fprintf(stream, "x=%e\n",cell->x);
  fprintf(stream, "y=%e\n",cell->y);

  fprintf(stream, "top_x=%e\n",cell->top_x);
  fprintf(stream, "top_y=%e\n",cell->top_y);
  fprintf(stream, "bot_x=%e\n",cell->bot_x);
  fprintf(stream, "bot_y=%e\n",cell->bot_y);

  fprintf(stream, "cell_width=%e\n",cell->cell_width);
  fprintf(stream, "cell_height=%e\n",cell->cell_height);

  fprintf(stream, "orientation=%d\n", cell->orientation);

  fprintf(stream, "color=%d\n", int_from_colour (cell->color));
  fprintf(stream, "clock=%d\n", cell->clock);

  fprintf(stream, "is_input=%d\n", cell->is_input);
  fprintf(stream, "is_output=%d\n", cell->is_output);
  fprintf(stream, "is_fixed=%d\n", cell->is_fixed);

  if(cell->label[strlen(cell->label)-1] != '\n')fprintf(stream, "label=%s\n", cell->label);
  if(cell->label[strlen(cell->label)-1] == '\n')fprintf(stream, "label=%s", cell->label);

  fprintf(stream, "number_of_dots=%d\n", cell->number_of_dots);


  // -- write the dots to the file -- //
  for(i = 0; i < cell->number_of_dots; i++){
    fprintf(stream, "[QDOT]\n");
    fprintf(stream, "x=%e\n",cell->cell_dots[i].x);
    fprintf(stream, "y=%e\n",cell->cell_dots[i].y);
    fprintf(stream, "diameter=%e\n",cell->cell_dots[i].diameter);
    fprintf(stream, "charge=%e\n",cell->cell_dots[i].charge);
    fprintf(stream, "spin=%e\n",cell->cell_dots[i].spin);
    fprintf(stream, "potential=%e\n",cell->cell_dots[i].potential);
    fprintf(stream, "[#QDOT]\n");
    }

  fprintf(stream, "[#QCELL]\n");
  }

gboolean gqcell_overlaps (GQCell *gqcL, GQCell *gqcR)
  {
  return
    gqcell_point_in_cell (gqcL, gqcR->top_x, gqcR->top_y) ||
    gqcell_point_in_cell (gqcL, gqcR->top_x, gqcR->bot_y) ||
    gqcell_point_in_cell (gqcL, gqcR->bot_x, gqcR->bot_y) ||
    gqcell_point_in_cell (gqcL, gqcR->bot_x, gqcR->top_y) ;
  }

gboolean gqcell_point_in_cell (GQCell *gqc, double x, double y)
  {
  return
    ((x >= gqc->top_x && x <= gqc->bot_x) && (y >= gqc->top_y && y <= gqc->bot_y)) ;
  }

GQCell *gqcell_new_from_stream (FILE *stream)
  {
  char *pszName = NULL, *pszValue = NULL ;

  //The file read buffer is 80 characters long. Any lines in the file longer than this will
  //result in unexpected outputs.
  char *buffer = NULL;

  GQCell *cell = g_object_new (TYPE_GQCELL, NULL);

  // -- The dot that is currently being read to -- //
  int current_dot = 0;

  // -- make sure the file is not NULL -- //
  if(NULL == stream)
    {
    printf("Cannot extract the design from a NULL file\n");
    printf("The pointer to the project file was passed as NULL to read_design()\n");
    g_object_unref (cell) ;
    return NULL;
    }
  
  DBG_GQC_READ (fprintf (stderr, "gqcell_new_from_stream:Still here (stream is not NULL)\n")) ;
  
  // -- read the first line in the file to the buffer -- //
  buffer = ReadLine (stream, 0) ;
  DBG_GQC_READ (fprintf (stderr, "Initial read:\n%s\n", buffer)) ;

  // -- check whether that was the end of the file -- //
  if(feof(stream))
    {
    //printf("Premature end of file reached your design may not have been loaded\n");
    g_object_unref (cell) ;
    return NULL;
    }

  while(strcmp(buffer, "[QCELL]") != 0)
    {
    buffer = ReadLine (stream, 0) ;
    DBG_GQC_READ (fprintf (stderr, "Looking for initial tag:\n%s\n", buffer)) ;

    if(feof(stream))
      {
      printf("Premature end of file reached your design may not have been loaded\n");
      printf("End of file reached while searching for the opening [QCELL] tag\n");
      g_object_unref (cell) ;
      return NULL;
      }
    }//while ![QCELL]

  cell->cell_dots = NULL;

  // -- read in the first option --//
  buffer = ReadLine (stream, 0) ;
  DBG_GQC_READ (fprintf (stderr, "Before main loop:\n%s\n", buffer)) ;

  // -- keep reading in lines of the file until the end tag is reached -- //
  while(strcmp(buffer, "[#QCELL]") != 0)
    {
    FindNameAndValue (buffer, &pszName, &pszValue) ;

    // -- Find and set the appropriate property of the cell from the buffer -- //
    if(!strncmp(pszName, "x", sizeof("x")))
      cell->x = atof(pszValue);
    else if(!strncmp(pszName, "y", sizeof("y")))
      cell->y = atof(pszValue);
    else if(!strncmp(pszName, "top_x", sizeof("top_x")))
      cell->top_x = atof(pszValue);
    else if(!strncmp(pszName, "top_y", sizeof("top_y")))
      cell->top_y = atof(pszValue);
    else if(!strncmp(pszName, "bot_x", sizeof("bot_x")))
      cell->bot_x = atof(pszValue);
    else if(!strncmp(pszName, "bot_y", sizeof("bot_y")))
      cell->bot_y = atof(pszValue);
    else if(!strncmp(pszName, "cell_width", sizeof("cell_width")))
      cell->cell_width = atof(pszValue);
    else if(!strncmp(pszName, "cell_height", sizeof("cell_height")))
      cell->cell_height = atof(pszValue);
    else if(!strncmp(pszName, "orientation", sizeof("orientation")))
      cell->orientation = atoi(pszValue);
    else if(!strncmp(pszName, "clock", sizeof("clock")))
      cell->clock = atoi(pszValue);
    else if(!strncmp(pszName, "color", sizeof("color")))
      cell->color = colour_from_int (atoi(pszValue));
    else if(!strncmp(pszName, "is_input", sizeof("is_input")))
      cell->is_input = atoi(pszValue);
    else if(!strncmp(pszName, "is_output", sizeof("is_output")))
      cell->is_output = atoi(pszValue);
    else if(!strncmp(pszName, "is_fixed", sizeof("is_fixed")))
      cell->is_fixed = atoi(pszValue);
    else if(!strncmp(pszName, "number_of_dots", sizeof("number_of_dots")))
      cell->number_of_dots = atoi(pszValue);
    else if(!strncmp(pszName, "label", sizeof("label")))
      cell->label = g_strdup (pszValue) ;
    // -- else check if this the opening tag for a QDOT -- //
    else if(strcmp(buffer, "[QDOT]") == 0)
      {
      // -- allocate the memory for the cell dots -- //
      if(cell->cell_dots == NULL)
        {
	if(cell->number_of_dots <= 0 )
          {
	  printf("Error attempting to load the dots into a cell: number_of_dots <=0\n");
	  printf("Possibly due to having [QDOT] definition before number_of_dots definition\n");
	  printf("The file has failed to load\n");
	  g_object_unref (cell) ;
	  return NULL;
          }
	cell->cell_dots = g_malloc(sizeof(qdot) * cell->number_of_dots);
	current_dot = 0;
        }

      buffer = ReadLine (stream, 0) ;
      DBG_GQC_READ (fprintf (stderr, "Found qdot:\n%s\n", buffer)) ;

      // -- extract all the data within the [QDOT] tags -- //
      while(strcmp(buffer, "[#QDOT]") != 0)
        {
        FindNameAndValue (buffer, &pszName, &pszValue) ;

	// -- Find and set the appropriate property of the cell from the buffer -- //
	if(!strncmp(pszName, "x", sizeof("x")))
		cell->cell_dots[current_dot].x = atof(pszValue);
	else if(!strncmp(pszName, "y", sizeof("y")))
		cell->cell_dots[current_dot].y = atof(pszValue);
	else if(!strncmp(pszName, "diameter", sizeof("diameter")))
		cell->cell_dots[current_dot].diameter = atof(pszValue);
	else if(!strncmp(pszName, "charge", sizeof("charge")))
		cell->cell_dots[current_dot].charge = atof(pszValue);
	else if(!strncmp(pszName, "spin", sizeof("spin")))
		cell->cell_dots[current_dot].spin = atof(pszValue);
	else if(!strncmp(pszName, "potential", sizeof("potenital")))
		cell->cell_dots[current_dot].potential = atof(pszValue);

	if(feof(stream))
          {
	  printf("Premature end of file reached while trying to locate the [#QDOT] tag\n");
          g_object_unref (cell) ;
	  return NULL;
	  }
        buffer = ReadLine (stream, 0) ;
        DBG_GQC_READ (fprintf (stderr, "Reading qdot params:\n%s\n", buffer)) ;
        }//while not [#QDOT]
      // -- Increment the dot counter and check if it is out of the bounds set by number_of_dots -- //
      if(++current_dot > cell->number_of_dots)
        {
	printf("There appear to be more [QDOTS] then the set number_of_dots in one of the cells\n");
	printf("The file has failed to load\n");
	g_object_unref (cell) ;
	return NULL;
	}
      }

    // -- Make sure that the terminating [#QCELL] tag was found prior to the end of the file -- //
    if(feof(stream))
      {
      printf("Premature end of file reached while trying to locate the [#QCELL] tag\n");
      g_object_unref (cell) ;
      return NULL;
      }
    buffer = ReadLine (stream, 0) ;
    DBG_GQC_READ (fprintf (stderr, "End of iteration:\n%s\n", buffer)) ;
    }

  // -- Make sure that the dots within the cell have been initalized -- //
  if(cell->cell_dots == NULL)
    {
    printf("QCELL appears to have been loaded without any dots\n");
    printf("Edit the project file and check for [QDOT] tags\n");
    g_object_unref (cell) ;
    return NULL;
    }
  
  DBG_GQC_READ (fprintf (stderr, "G_IS_OBJECT (cell) returns %s\n", G_IS_OBJECT (cell) ? "TRUE" : "FALSE")) ;

  return cell;
  }//read_design

static void FindNameAndValue (char *psz, char **ppszName, char **ppszValue)
  {
  int idx = 0 ;
  (*ppszName) = psz ;
  for (; psz[idx] != 0 ; idx++)
    if ('=' == psz[idx])
      {
      psz[idx] = 0 ;
      (*ppszValue) = &(psz[idx + 1]) ;
      }
    if ('\r' == psz[idx] || '\n' == psz[idx])
      psz[idx] = 0 ;
  }

static inline GdkColor *colour_from_int (int clr)
  {
  return
       RED == clr ? &clrRed :
      BLUE == clr ? &clrBlue :
    YELLOW == clr ? &clrYellow :
    ORANGE == clr ? &clrOrange :
     GREEN == clr ? &(clrClock[0]) :
    GREEN1 == clr ? &(clrClock[1]) :
    GREEN2 == clr ? &(clrClock[2]) : &(clrClock[3]) ;
  }

static inline int int_from_colour (GdkColor *clr)
  {
  return
         &clrRed == clr ? RED :
        &clrBlue == clr ? BLUE :
      &clrYellow == clr ? YELLOW :
      &clrOrange == clr ? ORANGE :
    &clrClock[0] == clr ? GREEN :
    &clrClock[1] == clr ? GREEN1 :
    &clrClock[2] == clr ? GREEN2 : GREEN3 ;
  }
