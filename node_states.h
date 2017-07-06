/* ************************************************************************** */
/** Descriptive File Name

  @Company
 B105 Electronic Systems Lab

  @File Name
    node_states.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _NODE_STATES_H    /* Guard against multiple inclusion */
#define _NODE_STATES_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

enum testbed_state {
	NODE_IDLE,
	NODE_SEND,
    NODE_RECEIVE
};
    
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
