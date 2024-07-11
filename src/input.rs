use raylib::RaylibHandle;
use crate::chip8::Chip8;

pub fn handle_input(chip8: &mut Chip8, rl: &RaylibHandle) {
    let keys = [
        raylib::consts::KeyboardKey::KEY_ONE,
        raylib::consts::KeyboardKey::KEY_TWO,
        raylib::consts::KeyboardKey::KEY_THREE,
        raylib::consts::KeyboardKey::KEY_FOUR,
        raylib::consts::KeyboardKey::KEY_Q,
        raylib::consts::KeyboardKey::KEY_W,
        raylib::consts::KeyboardKey::KEY_E,
        raylib::consts::KeyboardKey::KEY_R,
        raylib::consts::KeyboardKey::KEY_A,
        raylib::consts::KeyboardKey::KEY_S,
        raylib::consts::KeyboardKey::KEY_D,
        raylib::consts::KeyboardKey::KEY_F,
        raylib::consts::KeyboardKey::KEY_Z,
        raylib::consts::KeyboardKey::KEY_X,
        raylib::consts::KeyboardKey::KEY_C,
        raylib::consts::KeyboardKey::KEY_V,
    ];

    for (i, key) in keys.iter().enumerate() {
        if rl.is_key_down(*key) {
            chip8.key[i] = 1;
        } else {
            chip8.key[i] = 0;
        }
    }
}
