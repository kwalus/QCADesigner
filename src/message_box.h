#ifndef _MESSAGE_BOX_H_
#define _MESSAGE_BOX_H_

typedef enum
  {
  MB_OK = 0x1,
  MB_YES = 0x2,
  MB_NO = 0x4,
  MB_CANCEL = 0x8
  } MBButton ;

MBButton message_box (GtkWindow *parent, MBButton btns, char *pszTitle, char *pszFormat, ...) ;

#endif /* _MESSAGE_BOX_H_ */
