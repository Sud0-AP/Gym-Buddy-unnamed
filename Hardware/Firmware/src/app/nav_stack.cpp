#include "nav_stack.h"

NavStack::NavStack() : m_depth(0) {
    init();
}

void NavStack::init() {
    m_depth = 0;
    m_stack[0] = { SCREEN_HOME, 0, 0 };
    for (uint8_t i = 1; i < NAV_STACK_MAX_DEPTH; i++) {
        m_stack[i] = { SCREEN_HOME, 0, 0 };
    }
}

void NavStack::reset_to_home() {
    init();
}

bool NavStack::push(const NavigationState& state) {
    if (m_depth + 1 >= NAV_STACK_MAX_DEPTH) {
        return false;
    }
    m_depth++;
    m_stack[m_depth] = state;
    return true;
}

bool NavStack::pop() {
    if (m_depth == 0) {
        return false;
    }
    m_depth--;
    return true;
}

const NavigationState& NavStack::current() const {
    return m_stack[m_depth];
}

NavigationState& NavStack::current() {
    return m_stack[m_depth];
}

uint8_t NavStack::get_depth() const {
    return m_depth;
}

const NavigationState& NavStack::get_state(uint8_t index) const {
    if (index >= NAV_STACK_MAX_DEPTH) {
        return m_stack[0];
    }
    return m_stack[index];
}

void NavStack::set_current_selection(uint8_t selection_index) {
    m_stack[m_depth].selection_index = selection_index;
}

void NavStack::set_current_scroll(uint16_t scroll_offset) {
    m_stack[m_depth].scroll_offset = scroll_offset;
}
