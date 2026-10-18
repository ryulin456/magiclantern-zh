#include <dryos.h>
#include <property.h>
#include <cfn-generic.h>

// on M6II these are not CFn, but in main menus
GENERIC_GET_ALO
GENERIC_GET_HTP

/* mirrorless, no such option */
int get_mlu() { return 0; }
void set_mlu(int value) { return; }

/* kitor: not sure what to do? Leaving nerfed for now.
 * C.Fn III 3 has wheel direction options
 * C.Fn III 2 has button assignments
 */
int cfn_get_af_button_assignment() { return 0; }
void cfn_set_af_button(int value) { return; }
