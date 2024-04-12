#[allow(non_snake_case)]
fn X(opcode: u16) -> usize {
    ((opcode & 0x0F00) >> 8) as usize
}

#[allow(non_snake_case)]
fn Y(opcode: u16) -> usize {
    ((opcode & 0x00F0) >> 4) as usize
}

#[allow(non_snake_case)]
fn NN(opcode: u16) -> u8 {
    (opcode & 0x00FF) as u8
}

#[allow(non_snake_case)]
fn NNN(opcode: u16) -> u16 {
    opcode & 0x0FFF
}

pub struct Chip8 {
    pub memory: [u8; 4096],
    pub v: [u8; 16],
    pub i: u16,
    pub pc: u16,
    pub gfx: [u8; 64 * 32],
    pub delay_timer: u8,
    pub sound_timer: u8,
    pub stack: [u16; 16],
    pub sp: u16,
    pub key: [u8; 16],
}

const FONTSET: [u8; 80] = [
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
];

pub fn init_chip8(content: Vec<u8>) -> Chip8 {
    let mut chip8 = Chip8 {
        memory: [0; 4096],
        v: [0; 16],
        i: 0,
        pc: 0x200,
        gfx: [0; 64 * 32],
        delay_timer: 0,
        sound_timer: 0,
        stack: [0; 16],
        sp: 0,
        key: [0; 16],
    };

    for font in FONTSET.iter().enumerate() {
        chip8.memory[font.0] = *font.1;
    }

    for (i, byte) in content.iter().enumerate() {
        chip8.memory[i + 0x200] = *byte;
    }

    chip8
}

pub fn emulate_cycle(chip8: &mut Chip8) {
    let opcode = (chip8.memory[chip8.pc as usize] as u16) << 8
        | chip8.memory[(chip8.pc + 1) as usize] as u16;

    match opcode & 0xF000 {
        0x0000 => match opcode & 0x00FF {
            0x00E0 => {
                chip8.gfx = [0; 64 * 32];
                chip8.pc += 2;
            }

            0x00EE => {
                chip8.sp -= 1;
                chip8.pc = chip8.stack[chip8.sp as usize];
                chip8.pc += 2;
            }

            0x0000 => chip8.pc = NNN(opcode),
            _ => println!("Unknown opcode: {:04X}", opcode),
        },

        0xA000 => {
            chip8.i = NNN(opcode);
            chip8.pc += 2;
        }

        0xD000 => {
            let x = chip8.v[X(opcode)] as u16;
            let y = chip8.v[Y(opcode)] as u16;
            let height = opcode & 0x000F;
            let mut pixel: u8;

            chip8.v[0xF] = 0;

            for yline in 0..height {
                pixel = chip8.memory[(chip8.i + yline) as usize];
                for xline in 0..8 {
                    if (pixel & (0x80 >> xline)) != 0 {
                        if chip8.gfx[(x + xline + ((y + yline) * 64)) as usize] == 1 {
                            chip8.v[0xF] = 1;
                        }
                        chip8.gfx[(x + xline + ((y + yline) * 64)) as usize] ^= 1;
                    }
                }
            }

            chip8.pc += 2;
        }

        0x1000 => chip8.pc = opcode & 0x0FFF,

        0x2000 => {
            chip8.stack[chip8.sp as usize] = chip8.pc;
            chip8.sp += 1;
            chip8.pc = NNN(opcode);
        }

        0x3000 => {
            if chip8.v[X(opcode)] == NN(opcode) {
                chip8.pc += 4;
            } else {
                chip8.pc += 2;
            }
        }

        0x4000 => {
            if chip8.v[X(opcode)] != NN(opcode) {
                chip8.pc += 4;
            } else {
                chip8.pc += 2;
            }
        }

        0x5000 => {
            if chip8.v[X(opcode)] == chip8.v[Y(opcode)] {
                chip8.pc += 4;
            } else {
                chip8.pc += 2;
            }
        }

        0x6000 => {
            chip8.v[X(opcode)] = NN(opcode);
            chip8.pc += 2;
        }

        0x7000 => {
            chip8.v[X(opcode)] += NN(opcode);
            chip8.pc += 2;
        }

        0x8000 => match opcode & 0x000F {
            0x0000 => {
                chip8.v[X(opcode)] = chip8.v[Y(opcode)];
                chip8.pc += 2;
            }

            0x0001 => {
                chip8.v[X(opcode)] |= chip8.v[Y(opcode)];
                chip8.pc += 2;
            }

            0x0002 => {
                chip8.v[X(opcode)] &= chip8.v[Y(opcode)];
                chip8.pc += 2;
            }

            0x0003 => {
                chip8.v[X(opcode)] ^= chip8.v[Y(opcode)];
                chip8.pc += 2;
            }

            0x0004 => {
                let x = X(opcode);
                let y = Y(opcode);
                let sum = chip8.v[x] as u16 + chip8.v[y] as u16;
                chip8.v[0xF] = if sum > 0xFF { 1 } else { 0 };
                chip8.v[x] = (sum & 0xFF) as u8;
                chip8.pc += 2;
            }

            0x0005 => {
                let x = X(opcode);
                let y = Y(opcode);
                chip8.v[0xF] = if chip8.v[x] > chip8.v[y] { 1 } else { 0 };
                chip8.v[x] = chip8.v[x].wrapping_sub(chip8.v[y]);
                chip8.pc += 2;
            }

            0x0006 => {
                let x = X(opcode);
                chip8.v[0xF] = chip8.v[x] & 0x1;
                chip8.v[x] >>= 1;
                chip8.pc += 2;
            }

            0x0007 => {
                let x = X(opcode);
                let y = Y(opcode);
                chip8.v[0xF] = if chip8.v[y] > chip8.v[x] { 1 } else { 0 };
                chip8.v[x] = chip8.v[y].wrapping_sub(chip8.v[x]);
                chip8.pc += 2;
            }

            0x000E => {
                let x = X(opcode);
                chip8.v[0xF] = chip8.v[x] >> 7;
                chip8.v[x] <<= 1;
                chip8.pc += 2;
            }

            _ => println!("Unknown opcode: {:04X}", opcode),
        },

        0x9000 => {
            if chip8.v[X(opcode)] != chip8.v[Y(opcode)] {
                chip8.pc += 4;
            } else {
                chip8.pc += 2;
            }
        }

        0xB000 => chip8.pc = NNN(opcode) + chip8.v[0] as u16,

        0xC000 => {
            chip8.v[X(opcode)] = NN(opcode) & rand::random::<u8>();
            chip8.pc += 2;
        }

        0xE000 => match opcode & 0x00FF {
            0x009E => {
                if chip8.key[chip8.v[X(opcode)] as usize] != 0 {
                    chip8.pc += 4;
                } else {
                    chip8.pc += 2;
                }
            }

            0x00A1 => {
                if chip8.key[chip8.v[X(opcode)] as usize] == 0 {
                    chip8.pc += 4;
                } else {
                    chip8.pc += 2;
                }
            }

            _ => println!("Unknown opcode: {:04X}", opcode),
        },

        0xF000 => match opcode & 0x00FF {
            0x0007 => {
                chip8.v[X(opcode)] = chip8.delay_timer;
                chip8.pc += 2;
            }

            0x000A => {
                let mut key_press = false;

                for i in 0..16 {
                    if chip8.key[i] != 0 {
                        chip8.v[X(opcode)] = i as u8;
                        key_press = true;
                    }
                }

                if !key_press {
                    return;
                }

                chip8.pc += 2;
            }

            0x0015 => {
                chip8.delay_timer = chip8.v[X(opcode)];
                chip8.pc += 2;
            }

            0x0018 => {
                chip8.sound_timer = chip8.v[X(opcode)];
                chip8.pc += 2;
            }

            0x001E => {
                chip8.i += chip8.v[X(opcode)] as u16;
                chip8.pc += 2;
            }

            0x0029 => {
                chip8.i = chip8.v[X(opcode)] as u16 * 0x5;
                chip8.pc += 2;
            }

            0x0033 => {
                let x = X(opcode);
                chip8.memory[chip8.i as usize] = chip8.v[x] / 100;
                chip8.memory[(chip8.i + 1) as usize] = (chip8.v[x] / 10) % 10;
                chip8.memory[(chip8.i + 2) as usize] = chip8.v[x] % 10;
                chip8.pc += 2;
            }

            0x0055 => {
                let x = X(opcode);
                for i in 0..x {
                    chip8.memory[(chip8.i + i as u16) as usize] = chip8.v[i];
                }
                chip8.i += x as u16 + 1;
                chip8.pc += 2;
            }

            0x0065 => {
                let x = X(opcode);
                for i in 0..x {
                    chip8.v[i] = chip8.memory[(chip8.i + i as u16) as usize];
                }
                chip8.i += x as u16 + 1;
                chip8.pc += 2;
            }

            _ => println!("Unknown opcode: {:04X}", opcode),
        },

        _ => println!("Unknown opcode: {:04X}", opcode),
    }
}
