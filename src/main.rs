mod chip8;
mod input;

use raylib::core::audio::{RaylibAudio, Sound};
use raylib::prelude::*;

use std::thread;
use std::time::Duration;

const CUBE_SIZE: i32 = 10;
const SCREEN_WIDTH: i32 = 64 * CUBE_SIZE;
const SCREEN_HEIGHT: i32 = 32 * CUBE_SIZE;
const TARGET_FPS: u64 = 300;

fn main() {
    let file_name = std::env::args().nth(1).expect("No file provided");
    let content = std::fs::read(file_name).expect("Could not read file");

    let (mut rl, thread) = raylib::init()
        .size(SCREEN_WIDTH, SCREEN_HEIGHT)
        .title("Chip-8 Emulator")
        .build();

    let mut chip8 = chip8::init_chip8(content);

    let mut audio = RaylibAudio::init_audio_device();
    let bamboo = Sound::load_sound("bamboo.wav").unwrap();

    while !rl.window_should_close() {
        input::handle_input(&mut chip8, &rl);
        let mut d = rl.begin_drawing(&thread);
        d.clear_background(Color::BLACK);

        chip8::emulate_cycle(&mut chip8);

        if chip8.delay_timer > 0 {
            chip8.delay_timer -= 1;
        }

        if chip8.sound_timer > 0 {
            if chip8.sound_timer == 1 {
                audio.play_sound_multi(&bamboo);
            }
            chip8.sound_timer -= 1;
        }

        for y in 0..32 {
            for x in 0..64 {
                if chip8.gfx[y * 64 + x] == 1 {
                    d.draw_rectangle(
                        x as i32 * CUBE_SIZE,
                        y as i32 * CUBE_SIZE,
                        CUBE_SIZE,
                        CUBE_SIZE,
                        Color::WHITE,
                    );
                }
            }
        }

        thread::sleep(Duration::from_millis(1000 / TARGET_FPS));
    }
}
