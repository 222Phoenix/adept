//caps from kb activates scrolling
//did not add auto mouse layer to kb

#include QMK_KEYBOARD_H

bool scrolling = false;
bool ball_arrows = false;
bool ball_volume = false;
bool ball_zoom = false;

int16_t  delta_x = 0;
int16_t  delta_y = 0;

#define SCROLL_THRESHOLD_X 30
#define SCROLL_THRESHOLD_Y 30

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {

	    if (ball_arrows) {		
		if (mouse_report.x < 0) {
			SEND_STRING(SS_DELAY(200));
			tap_code(KC_LEFT);	
		}
		else if (mouse_report.x > 0) {
			SEND_STRING(SS_DELAY(200));
			tap_code(KC_RGHT);			
		}				
        mouse_report.h = 0;		
        mouse_report.v = 0;
        mouse_report.x = 0;
        mouse_report.y = 0;	
    }
	
	    if (ball_volume) {		
		if (mouse_report.y < 0) {       //wheel udlr is a too fast, just do ball scrolling
			SEND_STRING(SS_DELAY(100));
			tap_code(KC_VOLU);
		}
		else if (mouse_report.y > 0) {
			SEND_STRING(SS_DELAY(100));			
			tap_code(KC_VOLD);
		}	
        mouse_report.h = 0;		
        mouse_report.v = 0;
        mouse_report.x = 0;
        mouse_report.y = 0;	
    }	

	    if (ball_zoom) {	
//        pointing_device_set_cpi(200);		
		if (mouse_report.y < 0) {       //wheel udlr is a too fast, just do ball scrolling
			SEND_STRING(SS_DELAY(400));
			tap_code(KC_WH_U);
		}
		else if (mouse_report.y > 0) {
//        pointing_device_set_cpi(1000);	
			SEND_STRING(SS_DELAY(400));			
			tap_code(KC_WH_D);
		}	
        mouse_report.h = 0;		
        mouse_report.v = 0;
        mouse_report.x = 0;
        mouse_report.y = 0;	
    }

		if (scrolling) {
        delta_x += mouse_report.x;
		delta_y += mouse_report.y;
		mouse_report.x = mouse_report.y = 0;

		if (delta_x > SCROLL_THRESHOLD_X) {
			mouse_report.h = 1;
			delta_x = 0;
		} else if (delta_x < -SCROLL_THRESHOLD_X) {
			mouse_report.h = -1;
			delta_x = 0;
		}

		if (delta_y > SCROLL_THRESHOLD_Y) {
			mouse_report.v = -1;
			delta_y = 0;
		} else if (delta_y < -SCROLL_THRESHOLD_Y) {
			mouse_report.v = 1;
			delta_y = 0;
		}
    }

    else {

		if (host_keyboard_led_state().caps_lock) { //caps on, same as scrolling
        delta_x += mouse_report.x;
		delta_y += mouse_report.y;
		mouse_report.x = mouse_report.y = 0;

		if (delta_x > SCROLL_THRESHOLD_X) {
			mouse_report.h = 1;
			delta_x = 0;
		} else if (delta_x < -SCROLL_THRESHOLD_X) {
			mouse_report.h = -1;
			delta_x = 0;
		}

		if (delta_y > SCROLL_THRESHOLD_Y) {
			mouse_report.v = -1;
			delta_y = 0;
		} else if (delta_y < -SCROLL_THRESHOLD_Y) {
			mouse_report.v = 1;
			delta_y = 0;
		}
    
    }	
		if (!host_keyboard_led_state().caps_lock) { //caps off, regular mousing
			return mouse_report;
    }		
	
	
}	
		return mouse_report;
}

enum custom_keycodes {
  COPYTXT = SAFE_RANGE,
  OFF
};

typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_TAP,
    TD_DOUBLE_HOLD,
    TD_DOUBLE_SINGLE_TAP, // Send two single taps
    TD_TRIPLE_TAP,
    TD_TRIPLE_HOLD	
} td_state_t;
typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;

enum tapdance {
  MCLK,
  RCLK,
  LTAB,
  RTAB,
  CPST,
  ARRW,
  VOLU
};

td_state_t cur_dance  (tap_dance_state_t *state);
void mclk_finished    (tap_dance_state_t *state, void *user_data);
void mclk_reset       (tap_dance_state_t *state, void *user_data);
void rclk_finished    (tap_dance_state_t *state, void *user_data);
void rclk_reset       (tap_dance_state_t *state, void *user_data);
void ltb_finished     (tap_dance_state_t *state, void *user_data);
void ltb_reset        (tap_dance_state_t *state, void *user_data);
void rtb_finished     (tap_dance_state_t *state, void *user_data);
void rtb_reset        (tap_dance_state_t *state, void *user_data);
void cpst_finished    (tap_dance_state_t *state, void *user_data);
void cpst_reset       (tap_dance_state_t *state, void *user_data);
void arrw_finished    (tap_dance_state_t *state, void *user_data);
void arrw_reset       (tap_dance_state_t *state, void *user_data);
void volu_finished    (tap_dance_state_t *state, void *user_data);
void volu_reset       (tap_dance_state_t *state, void *user_data);

td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->interrupted || !state->pressed) return TD_SINGLE_TAP;
        // Key has not been interrupted, but the key is still held. Means you want to send a 'HOLD'.
        else return TD_SINGLE_HOLD;
    } else if (state->count == 2) {
        // TD_DOUBLE_SINGLE_TAP is to distinguish between typing "pepper", and actually wanting a double tap
        // action when hitting 'pp'. Suggested use case for this return value is when you want to send two
        // keystrokes of the key, and not the 'double tap' action/macro.
        if (state->interrupted) return TD_DOUBLE_SINGLE_TAP;
        else if (state->pressed) return TD_DOUBLE_HOLD;
        else return TD_DOUBLE_TAP;
    }
        // Assumes no one is trying to type the same letter three times (at least not quickly).
        // If your tap dance key is 'KC_W', and you want to type "www." quickly - then you will need to add
        // an exception here to return a 'TD_TRIPLE_SINGLE_TAP', and define that enum just like 'TD_DOUBLE_SINGLE_TAP'
    if (state->count == 3) {
        if (state->interrupted || !state->pressed) return TD_TRIPLE_TAP;
        else return TD_TRIPLE_HOLD;
    } else return TD_UNKNOWN;
}

static td_tap_t mclk_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void mclk_finished(tap_dance_state_t *state, void *user_data) {
    mclk_tap_state.state = cur_dance(state);
    switch (mclk_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_BTN3);
            break;
        case TD_SINGLE_HOLD:
		    pointing_device_set_cpi(200);
		    register_code(KC_LCTL);
			ball_zoom = true;
            break;
        case TD_DOUBLE_TAP:
		    tap_code16(C(S(KC_T)));	
            break;		
	    default:
            break;		
    }
}
void mclk_reset(tap_dance_state_t *state, void *user_data) {
    if (mclk_tap_state.state == TD_SINGLE_HOLD) {	
			ball_zoom = false;			
	        unregister_code(KC_LCTL);	
            pointing_device_set_cpi(800);			
    }
}

static td_tap_t rclk_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void rclk_finished(tap_dance_state_t *state, void *user_data) {
    rclk_tap_state.state = cur_dance(state);
    switch (rclk_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_BTN2);
            break;
        case TD_SINGLE_HOLD:
			scrolling = true;		
            break;
        case TD_DOUBLE_TAP:
		    tap_code16(C(KC_W));			
            break;
        case TD_TRIPLE_TAP:
		    layer_on(2);
            scrolling = true;
            break;				
	    default:
            break;		
    }
}
void rclk_reset(tap_dance_state_t *state, void *user_data) {
    if (rclk_tap_state.state == TD_SINGLE_HOLD) {			
			scrolling = false;
    }
}

static td_tap_t ltb_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void ltb_finished(tap_dance_state_t *state, void *user_data) {
    ltb_tap_state.state = cur_dance(state);
    switch (ltb_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_LEFT);
            break;
        case TD_SINGLE_HOLD:
            tap_code16(KC_LT);		
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_LT);
            break;
	    default:
            break;		
    }
}
void ltb_reset(tap_dance_state_t *state, void *user_data) {
    ltb_tap_state.state = TD_NONE;
}

static td_tap_t rtb_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void rtb_finished(tap_dance_state_t *state, void *user_data) {
    rtb_tap_state.state = cur_dance(state);
    switch (rtb_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_RGHT);
            break;
        case TD_SINGLE_HOLD:
            tap_code16(KC_GT);		
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_GT);
            break;
	    default:
            break;			
    }
}
void rtb_reset(tap_dance_state_t *state, void *user_data) {
    rtb_tap_state.state = TD_NONE;
}

static td_tap_t cpst_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void cpst_finished(tap_dance_state_t *state, void *user_data) {
    cpst_tap_state.state = cur_dance(state);
    switch (cpst_tap_state.state) {
        case TD_SINGLE_TAP:
			tap_code16(C(KC_C));
            break;
        case TD_SINGLE_HOLD:
			tap_code16(C(KC_V));
            break;
	    default:
            break;			
    }
}
void cpst_reset(tap_dance_state_t *state, void *user_data) {
    cpst_tap_state.state = TD_NONE;
}

static td_tap_t arrw_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void arrw_finished(tap_dance_state_t *state, void *user_data) {
    arrw_tap_state.state = cur_dance(state);
    switch (arrw_tap_state.state) {
        case TD_SINGLE_TAP:
		    tap_code16(KC_ESC);
            break;
        case TD_SINGLE_HOLD:
			ball_arrows = true;
            break;
	    default:
            break;			
    }
}
void arrw_reset(tap_dance_state_t *state, void *user_data) {
    if (arrw_tap_state.state == TD_SINGLE_HOLD) {
			ball_arrows = false;
    }
}

static td_tap_t volu_tap_state = {
  .is_press_action = true,
  .state = TD_NONE
};
void volu_finished(tap_dance_state_t *state, void *user_data) {
    volu_tap_state.state = cur_dance(state);
    switch (volu_tap_state.state) {
        case TD_SINGLE_TAP:
			tap_code16(KC_ESC);
            break;
        case TD_SINGLE_HOLD:
			layer_on(1);
			ball_volume = true;
            break;
	    default:
            break;			
    }
}
void volu_reset(tap_dance_state_t *state, void *user_data) {
    if (volu_tap_state.state == TD_SINGLE_HOLD) {
			ball_volume = false;
			layer_off(1);
    }
}

tap_dance_action_t tap_dance_actions[] = {
  [MCLK] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    mclk_finished,    mclk_reset),
  [RCLK] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    rclk_finished,    rclk_reset),
  [LTAB] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    ltb_finished,     ltb_reset),
  [RTAB] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    rtb_finished,     rtb_reset),
  [CPST] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    cpst_finished,    cpst_reset),
  [ARRW] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    arrw_finished,    arrw_reset),
  [VOLU] = ACTION_TAP_DANCE_FN_ADVANCED  (NULL,    volu_finished,    volu_reset)  
};

enum combo_events {
  RESET_COMBO,
  ESC_COMBO,      //not used
  SCR_COMBO,
  COMBO_LENGTH
};
uint16_t COMBO_LEN = COMBO_LENGTH; // remove the COMBO_COUNT define and use this instead!

const uint16_t PROGMEM reset_combo[]   = {C(A(KC_TAB)),  TD(RCLK),  COMBO_END};
const uint16_t PROGMEM esc_combo[]     = {TD(LTAB),      TD(RTAB),  COMBO_END};
const uint16_t PROGMEM scr_combo[]     = {TD(VOLU),      TD(RCLK),  COMBO_END};

combo_t key_combos[] = {
  [RESET_COMBO] = COMBO_ACTION(reset_combo),
  [ESC_COMBO]   = COMBO_ACTION(esc_combo),
  [SCR_COMBO]   = COMBO_ACTION(scr_combo), 
};

void process_combo_event(uint16_t combo_index, bool pressed) {
  switch(combo_index) {
    case RESET_COMBO:
      if (pressed) {
//        tap_code16(QK_BOOT);
          reset_keyboard();
      }
      break;
    case ESC_COMBO:
      if (pressed) {
        tap_code16(KC_ESC);
      }
      break;
    case SCR_COMBO:	
      if (pressed) {
		  layer_on(2);
          scrolling = true;
      }
      break;	  
	  
  }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	
    [0] = LAYOUT( C(A(KC_TAB)), DPI_CONFIG, TD(MCLK),   TD(RCLK), 
	              KC_BTN1,                              TD(VOLU)),
	
	[1] = LAYOUT( _______,      _______,    KC_LT,      KC_GT,  
	              _______,                              _______),
				  
	[2] = LAYOUT( OFF,          OFF,        OFF,        OFF,  
	              OFF,                                  OFF)				  
	
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case COPYTXT:
      if (record->event.pressed){
	      SEND_STRING(SS_DOWN(X_LCTL) SS_TAP(X_C) SS_DELAY(100) SS_TAP(X_L) SS_DELAY(50) SS_TAP(X_V) SS_DELAY(50) SS_TAP(X_A) SS_DELAY(50) SS_TAP(X_C) SS_UP(X_LCTL));
          }
      return false; break; 
    case OFF:
      if (record->event.pressed){
		  layer_off(2);
		  scrolling = false;
          }
      return false; break; 
            }
    return true;
}

