#pragma once

#ifdef __CPLUSPLUS
extern "C" {
#endif

struct dfa_t;

enum {
    /** Start state, should always be defined */
    DFA_STATE_START = 0,

    /** Reject state, cannot be defined, generated automatically */
    DFA_STATE_REJECT = -1,
};

/**
 * Client-implements input classifier function
 */
typedef bool (*dfa_input_classifier_t) (char c);

/**
 * Define a new DFA with a given name
 */
dfa_t* dfa_define(const char* name);

/**
 * Define a new DFA state
 *
 * \dfa                 DFA
 * \state               New state index
 * \classifier          Name of the input classifier predicate of type dfa_input_classifier_t
 * \next_state          Next state index to transition to if classifier predicate returns true
 */
int dfa_define_state_transition(dfa_t* dfa, int state, const char* classifier, int next_state);
int dfa_define_state(dfa_t* dfa, int state, const char** classifiers, int* next_states, size_t total_transitions);

/**
 * Mark either existing or new state as accept state.
 * Reaching this state will make DFA accept the input string.
 *
 * \dfa                 DFA
 * \state               State index to mark
 */
int dfa_define_accept_state(dfa_t* dfa, int state);

/**
 * Generate DFA function code
 *
 * \dfa                 DFA
 * \outputfd            File descriptor to emit C code
 */
int dfa_generate(dfa_t* dfa, int outputfd);

#ifdef __CPLUSPLUS
}
#endif
