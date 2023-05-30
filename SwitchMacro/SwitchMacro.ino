/**
    ■操作方法
    　上キーと下キーを同時押しすると「十字キー入力」と「左スティック入力」の切り替えができます。
    　左キーと右キーを同時押しするとマクロが起動します。
    　マクロ実行中に左キーと右キーを同時押しするとマクロを停止できます。
*/

#include <Arduboy2.h>
#include <SwitchControlLibrary.h>

Arduboy2 arduboy;

void macro_input(String input, int press_delay = 50, int release_delay = 500);
void macro_delay(int delay_time);

char *text;
byte x;
byte y;

boolean is_menu_mode = false;
boolean is_macro_mode = false;
boolean is_macro_cancel = false;
unsigned long loop_num;

#define MENU_ROW_NUM 6
#define COMMAND_NUM 6
String command_list[COMMAND_NUM];

#define COMMAND_A_BUTTON_REPEAT 0
#define COMMAND_YUGIOH_SURRENDER 1
#define COMMAND_XENOBLADE2_BOND_BLADE 2
#define COMMAND_XENOBLADE2_EPIC_COLLECT 3
#define COMMAND_XENOBLADE2_ARDURAN_LOOP 4
#define COMMAND_SMASHBROS_SPECTATOR 5

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define NUM_CHARS (sizeof(text) - 1)
#define X_MAX (WIDTH - (NUM_CHARS * CHAR_WIDTH) + 1)
#define Y_MAX (HEIGHT - CHAR_HEIGHT)

void setup() {
	arduboy.begin();
	arduboy.setFrameRate(15);

	x = (WIDTH / 2) - (NUM_CHARS * CHAR_WIDTH / 2);
	y = (HEIGHT / 2) - (CHAR_HEIGHT / 2);
	text = "@";

	// メニューの選択肢作成
	command_list[COMMAND_A_BUTTON_REPEAT] = F("A BUTTON REPEAT");
	command_list[COMMAND_YUGIOH_SURRENDER] = F("[YuGiOh]SURRENDER");
	command_list[COMMAND_XENOBLADE2_BOND_BLADE] = F("[Xeno2]BOND BLADE");
	command_list[COMMAND_XENOBLADE2_EPIC_COLLECT] = F("[Xeno2]EPIC COLLECT");
	command_list[COMMAND_XENOBLADE2_ARDURAN_LOOP] = F("[Spla3]NAWABATO");
	command_list[COMMAND_SMASHBROS_SPECTATOR] = F("[SmaSP]SPECTATOR");
	
	// Switchにつなぐ前に何かキーを送信しないと認識されないことがあるので、キー入力しておく。
	macro_input("A");
}

void loop() {
	if (!(arduboy.nextFrame())) {
		return;
	}

	arduboy.pollButtons();

	// ～～～～～～～～～～～～～～～～～
	// ～～～～～ デバッグ用 ～～～～～
	// ～～～～～～～～～～～～～～～～～
	/*
	    if (arduboy.pressed(UP_BUTTON)) {
		macro_input("LS→");
		return;
	    } else if (arduboy.pressed(DOWN_BUTTON)) {
		macro_input("LS←:1000,LS→:1000");
		return;
	    } else if (arduboy.pressed(LEFT_BUTTON)) {
		macro_input("LS←:1000");
		return;
	    } else if (arduboy.pressed(RIGHT_BUTTON)) {
		macro_input("LS→:1000");
		return;
	    }
	*/


	// ～～～～～～～～～～～～～～～～～
	// ～～～～～ メニュー処理 ～～～～～
	// ～～～～～～～～～～～～～～～～～
	static int menu_position = 0;
	int is_macro_start = false;
	if (!is_macro_mode && !is_macro_cancel) {
		int is_menu_start = false;
		if (!is_menu_mode && arduboy.justPressed(LEFT_BUTTON) && arduboy.justPressed(RIGHT_BUTTON)) {
			is_menu_mode = true;
			is_menu_start = true;
		} else if (is_menu_mode) {
			if ((arduboy.justPressed(LEFT_BUTTON) && arduboy.justPressed(RIGHT_BUTTON)) ||
			        arduboy.justPressed(A_BUTTON)) {
				is_menu_mode = false;
				return;
			}
		}

		if (is_menu_mode) {
			// メニュー表示
			if (is_menu_start) {
				// メニュー表示時の初期化処理
				// 今は特に何もナシ。
				is_menu_start = false;
			}

			static int menu_start = 0;
			static int menu_end = MENU_ROW_NUM;
			if (COMMAND_NUM < menu_end) {
				menu_end = COMMAND_NUM;
			}
			if (arduboy.justPressed(UP_BUTTON)) {
				if (0 < menu_position) {
					if (menu_position == menu_start) {
						menu_start--;
						menu_end--;
					}

					menu_position--;
				} else {
					menu_end = COMMAND_NUM;
					menu_start = menu_end - MENU_ROW_NUM;
					if (menu_start < 0) {
						menu_start = 0;
					}

					menu_position = COMMAND_NUM - 1;
				}
			}
			if (arduboy.justPressed(DOWN_BUTTON)) {
				if (menu_position < COMMAND_NUM - 1) {
					if (menu_position == menu_end - 1) {
						menu_start++;
						menu_end++;
					}

					menu_position++;
				} else {
					menu_start = 0;
					menu_end = menu_start + MENU_ROW_NUM;
					if (COMMAND_NUM < menu_end) {
						menu_end = COMMAND_NUM;
					}

					menu_position = 0;
				}
			}

			// メニューのリストを表示する。
			arduboy.clear();
			for (int i = menu_start; i < menu_end; i++) {
				int menu_row_y = 10 * i - 10 * menu_start;
				if (menu_position == i) {
					arduboy.setCursor(0, menu_row_y);
					arduboy.print("*");
				}
				arduboy.setCursor(10, menu_row_y);
				arduboy.print(command_list[i]);
			}
			arduboy.display();

			// Bボタンが押されたらマクロを実行する。
			if (arduboy.justPressed(B_BUTTON)) {
				is_menu_mode = false;
				is_macro_mode = true;
				is_macro_start = true;
			} else {
				return;
			}
		}
	}


	// ～～～～～～～～～～～～～～～～～～
	// ～～～～～ マクロ実行処理 ～～～～～
	// ～～～～～～～～～～～～～～～～～～
	if (is_macro_cancel) {
		is_macro_mode = false;
		if (!arduboy.pressed(LEFT_BUTTON) || !arduboy.pressed(RIGHT_BUTTON)) {
			is_macro_cancel = false;
		}
	}
	if (is_macro_mode) {
		if (is_macro_start) {
			is_macro_start = false;
			loop_num = 0;

			print_status("MACRO START");
			delay(1000);
		}

		// マクロ関数を呼び出す。
		loop_num++;
		if (999999999 < loop_num) {
			loop_num = 1;
		}

		if (menu_position == COMMAND_A_BUTTON_REPEAT) {
			// Aボタン連打マクロ実行
			command_a_button_repeat();
		} else if (menu_position == COMMAND_YUGIOH_SURRENDER) {
			// サレンダーマクロ実行
			command_yugioh_surrender();
		} else if (menu_position == COMMAND_XENOBLADE2_BOND_BLADE) {
			// コアクリスタル同調マクロ実行
			command_xenoblade2_bond_blade();
		} else if (menu_position == COMMAND_XENOBLADE2_EPIC_COLLECT) {
			// エピックコアクリスタル集めマクロ実行
			command_xenoblade2_epic_collect();
		} else if (menu_position == COMMAND_XENOBLADE2_ARDURAN_LOOP) {
			// アルドランループマクロ実行
			command_xenoblade2_arduran_loop();
		} else if (menu_position == COMMAND_SMASHBROS_SPECTATOR) {
			// 大観戦マクロ実行
			command_smashbros_spectator();
		} else {
			// マクロ関数未定義の場合はマクロ実行をキャンセルする。
			is_macro_cancel = true;

			print_status(F("MACRO NOT FOUND"));
			delay(1000);
		}

		return;
	}


	// ～～～～～～～～～～～～～～～～～～～～
	// ～～～～～ コントローラー処理 ～～～～～
	// ～～～～～～～～～～～～～～～～～～～～
	static boolean is_stick_mode = true;
	if (arduboy.justPressed(UP_BUTTON) && arduboy.justPressed(DOWN_BUTTON)) {
		// 十字キーとスティックの切り替え処理
		is_stick_mode = !is_stick_mode;
	}

	boolean is_change = false;
	if (arduboy.justPressed(UP_BUTTON) || arduboy.justReleased(UP_BUTTON)) {
		is_change = true;
	}

	if (arduboy.justPressed(DOWN_BUTTON) || arduboy.justReleased(DOWN_BUTTON)) {
		is_change = true;
	}

	if (arduboy.justPressed(LEFT_BUTTON) || arduboy.justReleased(LEFT_BUTTON)) {
		is_change = true;
	}

	if (arduboy.justPressed(RIGHT_BUTTON) || arduboy.justReleased(RIGHT_BUTTON)) {
		is_change = true;
	}

	if (is_change) {
		if (is_stick_mode) {
			int stick_x = 128;
			int stick_y = 128;

			if (arduboy.pressed(UP_BUTTON + DOWN_BUTTON)) {
				stick_y = 128;
			} else if (arduboy.pressed(UP_BUTTON)) {
				stick_y = 0;
			} else if (arduboy.pressed(DOWN_BUTTON)) {
				stick_y = 255;
			}

			if (arduboy.pressed(LEFT_BUTTON + RIGHT_BUTTON)) {
				stick_x = 128;
			} else if (arduboy.pressed(LEFT_BUTTON)) {
				stick_x = 0;
			} else if (arduboy.pressed(RIGHT_BUTTON)) {
				stick_x = 255;
			}
			SwitchControlLibrary().MoveLeftStick(stick_x, stick_y);
		} else {
			uint8_t hat_position = (uint8_t)Hat::CENTER;
			if (arduboy.pressed(UP_BUTTON + DOWN_BUTTON + LEFT_BUTTON + RIGHT_BUTTON)) {
				hat_position = (uint8_t)Hat::CENTER;
			} else if (arduboy.pressed(UP_BUTTON + LEFT_BUTTON)) {
				hat_position = (uint8_t)Hat::TOP_LEFT;
			} else if (arduboy.pressed(UP_BUTTON + RIGHT_BUTTON)) {
				hat_position = (uint8_t)Hat::TOP_RIGHT;
			} else if (arduboy.pressed(UP_BUTTON)) {
				hat_position = (uint8_t)Hat::TOP;
			} else if (arduboy.pressed(DOWN_BUTTON + LEFT_BUTTON)) {
				hat_position = (uint8_t)Hat::BOTTOM_LEFT;
			} else if (arduboy.pressed(DOWN_BUTTON + RIGHT_BUTTON)) {
				hat_position = (uint8_t)Hat::BOTTOM_RIGHT;
			} else if (arduboy.pressed(DOWN_BUTTON)) {
				hat_position = (uint8_t)Hat::BOTTOM;
			} else if (arduboy.pressed(LEFT_BUTTON)) {
				hat_position = (uint8_t)Hat::LEFT;
			} else if (arduboy.pressed(RIGHT_BUTTON)) {
				hat_position = (uint8_t)Hat::RIGHT;
			}
			SwitchControlLibrary().MoveHat(hat_position);
		}
	}

	if (arduboy.justPressed(A_BUTTON)) {
		SwitchControlLibrary().PressButtonB();
	} else if (arduboy.justReleased(A_BUTTON)) {
		SwitchControlLibrary().ReleaseButtonB();
	}

	if (arduboy.justPressed(B_BUTTON)) {
		SwitchControlLibrary().PressButtonA();
	} else if (arduboy.justReleased(B_BUTTON)) {
		SwitchControlLibrary().ReleaseButtonA();
	}


	// ～～～～～～～～～～～～～～～～～～～～～～～～～～
	// ～～～～～ おまけのコントローラー表示処理 ～～～～～
	// ～～～～～～～～～～～～～～～～～～～～～～～～～～
	if (arduboy.pressed(UP_BUTTON)) {
		if (y > 0) {
			y--;

		}
	}

	if (arduboy.pressed(DOWN_BUTTON)) {
		if (y < Y_MAX) {
			y++;
		}
	}

	if (arduboy.pressed(LEFT_BUTTON)) {
		if (x > 0) {
			x--;
		}
	}

	if (arduboy.pressed(RIGHT_BUTTON)) {
		if (x < X_MAX) {
			x++;
		}
	}

	if (arduboy.pressed(B_BUTTON)) {
		text = "A";
	} else if (arduboy.pressed(A_BUTTON)) {
		text = "B";
	} else {
		text = "@";
	}

	arduboy.clear();
	arduboy.setCursor(x, y);
	arduboy.print(text);
	arduboy.display();
}

/**
    ステータス表示処理
*/
void print_status(String new_status) {
	static String status_text = "";
	if (new_status != "") {
		status_text = new_status;
	}
	arduboy.clear();

	if (is_macro_mode) {
		char buf[20];
		arduboy.setCursor(0, 0);
		sprintf(buf, "LOOP:%09lu", loop_num);
		arduboy.print(buf);
	}

	arduboy.setCursor(15, 30);
	arduboy.print(status_text);
	arduboy.display();
}

/**
    引数keyに以下の文字列を渡すとキー入力を行うことができます。
    ■ボタン
    　A、B、X、Y、L、ZL、R、ZR、-、+、HOME、CAPTURE
    ■十字キー
    　↑、↓、→、←
    ■左スティック
    　LS↑、LS↓、LS→、LS←
    　スティック押し込みは「LS凹」です。
    ■右スティック
    　RS↑、RS↓、RS→、RS←
    　スティック押し込みは「RS凹」です。
*/
void macro_input(String input, int press_delay = 50, int release_delay = 500) {
	//print_status("");

	// 表記ゆれ吸収
	input.replace("、", ",");
	input.toUpperCase();

	String key;
	int offset = 0;
	int hit;
	do {
		if (arduboy.pressed(LEFT_BUTTON) && arduboy.pressed(RIGHT_BUTTON)) {
			is_macro_cancel = true;
		}
		if (is_macro_cancel) {
			return;
		}
		
		hit = input.indexOf(",", offset);
		if (hit != 0) {
			key = input.substring(offset, hit);
			offset = hit + 1;
		} else {
			key = input.substring(offset);
			break;
		}

		int press_delay_hit = key.indexOf(":");
		if (press_delay_hit != -1) {
			String press_delay_str = key.substring(press_delay_hit + 1);
			key = key.substring(0, press_delay_hit);

			if (press_delay_str.length() != 0) {
				press_delay = press_delay_str.toInt();
			}
		}

		if (key.length() == 0) {
			continue;
		}

		if (String(F("A")).equals(key)) {
			SwitchControlLibrary().PressButtonA();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonA();
			macro_delay(release_delay);
		} else if (String(F("B")).equals(key)) {
			SwitchControlLibrary().PressButtonB();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonB();
			macro_delay(release_delay);
		} else if (String(F("X")).equals(key)) {
			SwitchControlLibrary().PressButtonX();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonX();
			macro_delay(release_delay);
		} else if (String(F("Y")).equals(key)) {
			SwitchControlLibrary().PressButtonY();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonY();
			macro_delay(release_delay);

		} else if (String(F("L")).equals(key)) {
			SwitchControlLibrary().PressButtonL();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonL();
			macro_delay(release_delay);
		} else if (String(F("ZL")).equals(key)) {
			SwitchControlLibrary().PressButtonZL();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonZL();
			macro_delay(release_delay);
		} else if (String(F("R")).equals(key)) {
			SwitchControlLibrary().PressButtonR();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonR();
			macro_delay(release_delay);
		} else if (String(F("ZR")).equals(key)) {
			SwitchControlLibrary().PressButtonZR();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonZR();
			macro_delay(release_delay);

		} else if (String(F("-")).equals(key)) {
			SwitchControlLibrary().PressButtonMinus();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonMinus();
			macro_delay(release_delay);
		} else if (String(F("+")).equals(key)) {
			SwitchControlLibrary().PressButtonPlus();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonPlus();
			macro_delay(release_delay);
		} else if (String(F("HOME")).equals(key)) {
			SwitchControlLibrary().PressButtonHome();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonHome();
			macro_delay(release_delay);
		} else if (String(F("CAPTURE")).equals(key)) {
			SwitchControlLibrary().PressButtonCapture();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonCapture();
			macro_delay(release_delay);

		} else if (String(F("↑")).equals(key)) {
			SwitchControlLibrary().MoveHat((uint8_t)Hat::TOP);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveHat((uint8_t)Hat::CENTER);
			macro_delay(release_delay);
		} else if (String(F("↓")).equals(key)) {
			SwitchControlLibrary().MoveHat((uint8_t)Hat::BOTTOM);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveHat((uint8_t)Hat::CENTER);
			macro_delay(release_delay);
		} else if (String(F("←")).equals(key)) {
			SwitchControlLibrary().MoveHat((uint8_t)Hat::LEFT);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveHat((uint8_t)Hat::CENTER);
			macro_delay(release_delay);
		} else if (String(F("→")).equals(key)) {
			SwitchControlLibrary().MoveHat((uint8_t)Hat::RIGHT);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveHat((uint8_t)Hat::CENTER);
			macro_delay(release_delay);

		} else if (String(F("LS凹")).equals(key)) {
			SwitchControlLibrary().PressButtonLClick();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonLClick();
			macro_delay(release_delay);
		} else if (String(F("RS凹")).equals(key)) {
			SwitchControlLibrary().PressButtonRClick();
			macro_delay(press_delay);
			SwitchControlLibrary().ReleaseButtonRClick();
			macro_delay(release_delay);

		} else if (String(F("LS↑")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(128, 0);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS↓")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(128, 255);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS←")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(0, 128);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS→")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(255, 128);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS↑←")).equals(key) || String(F("LS←↑")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(0, 0);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS↑→")).equals(key) || String(F("LS→↑")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(255, 0);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS↓←")).equals(key) || String(F("LS←↓")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(0, 255);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("LS↓→")).equals(key) || String(F("LS→↓")).equals(key)) {
			SwitchControlLibrary().MoveLeftStick(255, 255);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveLeftStick(128, 128);
			macro_delay(release_delay);

		} else if (String(F("RS↑")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(128, 0);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS↓")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(128, 255);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS←")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(0, 128);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS→")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(255, 128);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS↑←")).equals(key) || String(F("RS←↑")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(0, 0);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS↑→")).equals(key) || String(F("RS→↑")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(255, 0);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS↓←")).equals(key) || String(F("RS←↓")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(0, 255);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);
		} else if (String(F("RS↓→")).equals(key) || String(F("RS→↓")).equals(key)) {
			SwitchControlLibrary().MoveRightStick(255, 255);
			macro_delay(press_delay);
			SwitchControlLibrary().MoveRightStick(128, 128);
			macro_delay(release_delay);

		} else {
			boolean is_number = true;
			for (int i = 0; i < key.length(); i++) {
				if (!isDigit(key.charAt(i))) {
					is_number = false;
					break;
				}
			}
			if (is_number) {
				macro_delay(key.toInt());
			}
		}
	} while (hit != -1);
}

/**
    マクロ実行時のdelay処理です。
    マクロキャンセル処理付きです。
*/
void macro_delay(int delay_time) {
	//print_status("");

	int delay_split = 500;
	int loop_num = delay_time / delay_split;
	for (int i = 0; i < loop_num; i++) {
		if (arduboy.pressed(LEFT_BUTTON) && arduboy.pressed(RIGHT_BUTTON)) {
			is_macro_cancel = true;
		}
		if (is_macro_cancel) {
			return;
		}

		delay(delay_split);
	}

	delay(delay_time - (loop_num * delay_split));
}

void int_log(int value) {
	arduboy.clear();
	char buf[20];
	arduboy.setCursor(0, 0);
	sprintf(buf, "VALUE:%d", value);
	arduboy.print(buf);
	arduboy.display();

	delay(3000);
}


// Aボタン連打マクロ
void command_a_button_repeat() {
	print_status(F("AAAAAAAAAAA"));
	macro_input("A", 50, 50);
}


// [遊戯王]サレンダーマクロ
void command_yugioh_surrender() {
	print_status(F("DP MODE START!!"));
	macro_delay(2000);

	// キャラ選択
	print_status(F("CHARA SELECT"));
	macro_input(F("A"));

	// デッキ選択
	print_status(F("DECK SELECT"));
	macro_input(F("A"));

	// ロード
	print_status(F("LOADING"));
	macro_delay(10000);

	// じゃんけん + 先攻後攻選択
	print_status(F("RPS"));
	for (int i = 0; i <= 10; i++) {
		macro_input(F("A"));
	}

	//ロード
	print_status(F("LOADING"));
	macro_delay(10000);

	// ディエル画面

	// 相手ターンかもしれないのでしばらく待つ
	print_status(F("WAIT"));
	// macro_delay(60000);
	macro_delay(5000);

	// サレンダーのダイアログが出ているかもしれないので何も出てない状態にリセットする。
	print_status(F("FIELD RESET"));
	macro_input(F("B,ZR,1000,B"));

	// カーソルが移動しているかもしれないので自分のデッキまで移動させる。
	macro_input(F("↑,R,↑,→"));

	// サレンダーする。
	print_status(F("SURRENDER"));
	macro_input(F("A,A,←,A"));

	// 敗北画面
	print_status(F("LOSE SCREEN"));
	macro_input(F("1000,A,1000,A,A,A"));

	print_status(F("COMPLETE!"));
	macro_delay(2000);
}


// [ゼノブレイド2]コアクリスタル同調マクロ
void command_xenoblade2_bond_blade() {
	print_status(F("CRYSTAL SELECT"));
	macro_input(F("A,250,A,250,A,250"));

	print_status(F("BOOSTER SELECT"));
	macro_input(F("A,2000,↑,A"));

	print_status(F("BOND BLADE"));
	macro_input("4000,A,B");

	print_status(F("COMPLETE!"));
	macro_delay(500);
}


// [ゼノブレイド2]エピック集め
void command_xenoblade2_epic_collect() {
	print_status(F("WAITING FOR EPIC"));
	if (loop_num == 1) {
		macro_input(F("R,A,8000,RS凹"));
	}
	macro_input(F("LS→:10000,LS↑→:1000"));
}


// [ゼノブレイド2]アルドランループ
// [Splatoon3]ナワバトラーランク上げ
// デッキ選択画面でスタート
void command_xenoblade2_arduran_loop() {
  print_status(F("NAME"));
	macro_input(F("A,A,A,A,A,A,A,A,A,A,A,A,4000"));
  
  print_status(F("DRAW"));
  macro_input(F("A,4000"));

  print_status(F("PASS"));
  for (int i = 0; i < 12; i++) {
    macro_input(F("↑,A,A,7000"));
  }

  print_status(F("RESULT"));
  macro_input(F("3000,A,A,A,A,A,A,A,4000"));


  /*
	print_status(F("SKIP TRAVEL"));
	macro_input(F("+,1000,→,A,500,↓,A,500,A,1500,↓,↓,A,A,11000"));

	print_status(F("MOVE"));
	// 右スティックを使うとロードの具合の関係なのかずれが出てくるので、左スティックのみで移動させる。
	// 右スティックを操作しないと横移動時に補正が動いてしまうので、少しだけ操作して補正を解除する。
	macro_input(F("RS↑,LS↑:5500,LS←:4500,LS↑:1800,LS←:800,LS↑:1800,LS←:1900"));

	print_status(F("WAKE UP ARDURAN"));
	macro_input(F("A,500,A,5000,RS→:700"));

	print_status(F("BATTLE"));
	macro_input(F("R,A,8000,RS凹"));
	
	// 60秒待つ
	for (int i = 0; i < 6; i++) {
		macro_input(F("LS→:9000,LS↑→:1000"));
	}

	print_status(F("CHAIN ATTACK"));
	macro_input(F("+,7000"));

	// エルマ
	macro_input(F("X,1400,B,5000"));
	
	// ヒカリ
	macro_input(F("X,1380,B,5000"));

	// ハナJD
	macro_input(F("B,1650,B,500,B,4000"));

	print_status(F("COMPLETE!"));
	macro_input(F("B,B,B"));
	macro_delay(6000);
	macro_input(F("LS↑→:1100,LS←:1500"));

	if (loop_num % 10 == 0) {
		// 長時間動かしていると動かなくなってたりするのでセーブしておく。
		macro_input(F("-,2000,A,1000,A,↑,A,1000,+,3000"));
	}
  */
}

// [大乱闘スマッシュブラザーズSP]大観戦ポイント稼ぎマクロ
void command_smashbros_spectator() {
	print_status(F("SPECTATOR"));
	if (loop_num == 1) {
		arduboy.initRandomSeed();
	}
	int num = random(1, 5);
	for (int i = 0; i < num; i++) {
		macro_input(F("→"));
	}
	macro_input(F("A,3000"));
}
