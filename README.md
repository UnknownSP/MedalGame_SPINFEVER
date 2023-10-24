# MedalGame_SPINFEVER

KONAMI様のスピンフィーバーのハードウェアを基にして、回路及びプログラムを作成し動作させたものです。

## 動作例

https://github.com/UnknownSP/MedalGame_SPINFEVER/assets/39638661/7b45e147-6ebf-4b3e-9ea2-101945382abf

## 回路

![SPIN_cercuit1](https://github.com/UnknownSP/MedalGame_SPINFEVER/assets/39638661/bea52022-4306-4ca7-a0ae-a35ed05baae9)

![SPIN_cercuit2](https://github.com/UnknownSP/MedalGame_SPINFEVER/assets/39638661/053a2d4b-c0dc-45b9-88e2-51528b0343db)

## 仕様

stm32f401_SPINFEVER がマスタであり、全ての動作の管理を行っています。
PICは分割された各部分の処理を行っており、以下のように分担されています。

- type1
  - センターバンパー4つの処理 
- type2 
  - アウトブロックの処理 (1基板で4つ)
- type4
  - 発射メカとアウトエリア、JPCリフタの処理 (1基板で1組ずつ)
- type6
  - JPC抽選機の処理 (1基板で1つ)
- type8
  - センターモータの処理   


