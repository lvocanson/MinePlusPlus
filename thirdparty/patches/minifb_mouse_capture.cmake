set(FILE src/windows/WinMiniFB.c)
file(READ ${FILE} CONTENT)

string(FIND "${CONTENT}" "WM_CAPTURECHANGED" APPLIED)
if (NOT APPLIED EQUAL -1)
	return()
endif()

function(replace_once ANCHOR REPLACEMENT)
	string(FIND "${CONTENT}" "${ANCHOR}" POSITION)
	if (POSITION EQUAL -1)
		message(FATAL_ERROR "minifb_mouse_capture: anchor not found in ${FILE}")
	endif()
	string(REPLACE "${ANCHOR}" "${REPLACEMENT}" CONTENT "${CONTENT}")
	set(CONTENT "${CONTENT}" PARENT_SCOPE)
endfunction()

replace_once(
[=[
                    window_data->mouse_button_status[button] = is_pressed;
                    kCall(mouse_btn_func, button, window_data->mod_keys, is_pressed);
]=]
[=[
                    window_data->mouse_button_status[button] = is_pressed;
                    if (is_pressed) {
                        SetCapture(hWnd);
                    }
                    else {
                        int any_pressed = 0;
                        for (int i = 0; i < MFB_MAX_MOUSE_BUTTONS; ++i) {
                            any_pressed |= window_data->mouse_button_status[i];
                        }
                        if (!any_pressed) {
                            ReleaseCapture();
                        }
                    }
                    kCall(mouse_btn_func, button, window_data->mod_keys, is_pressed);
]=])

# TODO: the synthesized releases use the last known mouse position, a UI element under it may register a click
replace_once(
[=[
        case WM_MOUSEWHEEL:
]=]
[=[
        case WM_CAPTURECHANGED:
            if (window_data && (HWND) lParam != hWnd) {
                for (int i = 0; i < MFB_MAX_MOUSE_BUTTONS; ++i) {
                    if (window_data->mouse_button_status[i]) {
                        window_data->mouse_button_status[i] = 0;
                        kCall(mouse_btn_func, (mfb_mouse_button) i, translate_mod(), false);
                    }
                }
            }
            break;

        case WM_MOUSEWHEEL:
]=])

file(WRITE ${FILE} "${CONTENT}")
