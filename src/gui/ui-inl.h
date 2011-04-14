#include "util.h"

inline int MyMessageBox(const wxString& message, const wxString& caption, int style, wxWindow* parent, int x, int y)
{
#ifdef GUI
    if (!fDaemon)
        return wxMessageBox(message, caption, style, parent, x, y);
#endif
    printf("wxMessageBox %s: %s\n", string(caption).c_str(), string(message).c_str());
    fprintf(stderr, "%s: %s\n", string(caption).c_str(), string(message).c_str());
    return wxOK;
}

#define wxMessageBox MyMessageBox
