#pragma once

#include "nav_state.h"

#define NAV_STACK_MAX_DEPTH 8

class NavStack {
public:
    NavStack();

    void init();
    void reset_to_home();

    bool push(const NavigationState& state);
    bool pop();

    const NavigationState& current() const;
    NavigationState& current();

    uint8_t get_depth() const;
    const NavigationState& get_state(uint8_t index) const;

    void set_current_selection(uint8_t selection_index);
    void set_current_scroll(uint16_t scroll_offset);

private:
    NavigationState m_stack[NAV_STACK_MAX_DEPTH];
    uint8_t m_depth;
};
