use crate::chip8::Chip8;
use raylib::RaylibHandle;

pub fn handle_input(chip8: &mut Chip8, rl: &RaylibHandle) {
    let keys = [
        raylib::consts::KeyboardKey::KEY_X,
        raylib::consts::KeyboardKey::KEY_A,
        raylib::consts::KeyboardKey::KEY_S,
        raylib::consts::KeyboardKey::KEY_D,
        raylib::consts::KeyboardKey::KEY_Q,
        raylib::consts::KeyboardKey::KEY_W,
        raylib::consts::KeyboardKey::KEY_E,
        raylib::consts::KeyboardKey::KEY_Z,
        raylib::consts::KeyboardKey::KEY_C,
        raylib::consts::KeyboardKey::KEY_V,
        raylib::consts::KeyboardKey::KEY_R,
        raylib::consts::KeyboardKey::KEY_T,
        raylib::consts::KeyboardKey::KEY_Y,
        raylib::consts::KeyboardKey::KEY_U,
        raylib::consts::KeyboardKey::KEY_I,
        raylib::consts::KeyboardKey::KEY_O,
    ];

    for (i, key) in keys.iter().enumerate() {
        if rl.is_key_down(*key) {
            chip8.key[i] = 1;
        } else {
            chip8.key[i] = 0;
        }
    }
}
