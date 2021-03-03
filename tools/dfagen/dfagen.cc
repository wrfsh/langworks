#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include <vector>
#include <queue>
#include <map>
#include <set>

#include "dfagen.h"

struct dfa_state_transition_t {
    /* Transition input classifier function name */
    const char* classifier;

    /* Next DFA state if classifier triggers */
    int next_state;
};

struct dfa_state_t {
    /* Accept state? */
    bool is_accept = false;

    /* All transitions from this state */
    std::vector<dfa_state_transition_t> transitions;
};

struct dfa_t {
    /* DFA human-readable name */
    const char* name;

    /* Map of dfa states */
    std::map<int, dfa_state_t> states;

    explicit dfa_t(const char* _name) : name(_name) {};
};

dfa_t* dfa_define(const char* name)
{
    dfa_t* dfa = new dfa_t(name);
    return dfa;
}

int dfa_define_state_transition(dfa_t* dfa, int state, const char* classifier, int next_state)
{
    if (!dfa) {
        return -EINVAL;
    }

    dfa->states[state].transitions.push_back({
            .classifier = classifier,
            .next_state = next_state,
    });
    
    return 0;
}

int dfa_define_state(dfa_t* dfa, int state, const char** classifiers, int* next_states, size_t total_transitions)
{
    if (!dfa) {
        return -EINVAL;
    }

    if (total_transitions != 0 && (!classifiers || !next_states)) {
        return -EINVAL;
    }

    if (dfa->states.find(state) != dfa->states.end()) {
        return -EBUSY;
    }

    for (size_t i = 0; i < total_transitions; ++i) {
        int error = dfa_define_state_transition(dfa, state, classifiers[i], next_states[i]);
        if (error) {
            return error;
        }
    }

    return 0;
}

int dfa_define_accept_state(dfa_t* dfa, int state)
{
    if (!dfa) {
        return -EINVAL;
    }

    dfa->states[state].is_accept = true;
    return 0;
}

static int emit(int fd, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vdprintf(fd, fmt, args);
    va_end(args);

    return res >= 0 ? 0 : res;
}

int dfa_generate(dfa_t* dfa, int outputfd)
{
    int error = 0;

    if (!dfa || outputfd < 0) {
        return -EINVAL;
    }

    /* Sanity check: start state needs to have non-zero transitions */
    auto p = dfa->states.find(DFA_STATE_START);
    if (p == dfa->states.end() || p->second.transitions.empty()) {
        return -EINVAL;
    }

    error |= emit(outputfd, "bool match_%s(const char* input, size_t len) {", dfa->name);
    error |= emit(outputfd, "const char* end = input + len;");

    std::set<int> visited;
    std::queue<int> q;
    q.push(DFA_STATE_START);

    while (!q.empty()) {
        int index = q.front();
        q.pop();

        if (visited.find(index) != visited.end()) {
            continue;
        }

        const dfa_state_t& state = dfa->states[index];
        error |= emit(outputfd, "S%u:", index);

        if (state.is_accept) {
            /* Accept states shouldn't have any transitions */
            if (!state.transitions.empty()) {
                return -EINVAL;
            }

            error |= emit(outputfd, "return true;");
        } else {
            error |= emit(outputfd, "if (input == end) { return false; }");
            for (const dfa_state_transition_t& t : state.transitions) {
                error |= emit(outputfd, "if (%s(*input)) { ++input; goto S%u; }", t.classifier, t.next_state);
                q.push(t.next_state);
            }

            error |= emit(outputfd, "return false;");
        }

        visited.insert(index);
    }

    error |= emit(outputfd, "}");
    return error;
}
