# Mergen

Mergen is a **real-time artificial intelligence**, which shall:

- serve as an operating system for any kind of robot.
- process and store **vision**, **audio** and **touch** as its input data.
- simulate the [**Reward System**](https://en.wikipedia.org/wiki/Reward_system) of the human brain.
- simulate both the [**Conscious**](https://en.wikipedia.org/wiki/Consciousness) and
  the [**Unconscious**](https://en.wikipedia.org/wiki/Unconscious_mind) parts of the human brain.
- produce **speech**, **movement** commands and probably **imagination** as its output data.

Mergen is intended to become a robot which can think more logically and wisely than human beings,
and potentially help them have better lives and consequently a better world(s)!

Emotions cloud human beings' judgements! Mergen won't be making mistakes the way human beings do
out of their [*wishful thinking*](https://en.wikipedia.org/wiki/Wishful_thinking)
neither in its judgements nor its decision-making.

> Mergen was named after a [*Turkic* god](https://en.wikipedia.org/wiki/Mergen) of abundance and wisdom!

Mergen is being written in **C++ 20** and is temporarily mounted on Android.
The library located in the [/cpp/](cpp) is intended to be mounted on real robots with a Unix-like operating system.

## Components

|      Codename      | Description                                 |
|:------------------:|:--------------------------------------------|
| [**AUD**](cpp/aud) | controlling **Auditory** inputs and outputs |
| [**HPT**](cpp/hpt) | controlling **Touch** (haptic) inputs       |
| [**MOV**](cpp/mov) | controlling **Movements**                   |
| [**REW**](cpp/rew) | simulation of the **Reward System**         |
| [**SCM**](cpp/scm) | simulation of the **Subconscious Mind**     |
| [**VIS**](cpp/vis) | controlling **Visual** inputs (and outputs) |

## History

#### Theorem

At 21 August 2020, I wanted to programme a simple Android app based on [**logical theorems**](
https://en.wikipedia.org/wiki/Theorem) using which one can enter a sentence as a theorem and numerous chance factors
below it which can make it *True* or *False*.
The app had to calculate their mathematical mean and say, for example:

> There is 20.4657% chance that **a god** exists!

This idea gradually turned into a robot who could do the exact same thing!

#### Mergen I (v0.1)

As I was getting more and more familiar with the machine learning concepts, I wanted to programme an Android app whose
programming language was merely Python using Chaquopy, but since it had a weird bothersome license,
I decided to forget about using Python in Android. Then I decided to try [ML Kit](
https://developers.google.com/ml-kit), but it had no use to this project either!
I wanted to name the robot I had planned to create "Horus", but then I found a better name for it!

The first version of *Mergen*, started at **13 December 2020**, was intended to be a
[natural language processing (NLP)](https://en.wikipedia.org/wiki/Natural_language_processing) bot,
something like ChatGPT today. In other words, it was going to simulate brain of a **mature human being**.

It was intended to process **only digital texts** in a server using pure Python, and the server had to
communicate with an Android client app.
But I concluded that such kind of robot has little "actual" use to human beings;
therefore I cancelled its development and pursued a better method in Mergen II, see below.

#### Mergen II (v0.2)

The second version of Mergen, started at 10-12 January 2021, was intended to primarily process audio,
I also called it the [**Pronouncer**](https://github.com/fulcrum6378/pronouncer).
It was a mixture of a [Speech Recognition](https://en.wikipedia.org/wiki/Speech_recognition) program and a
[Text-To-Speech (TTS)](https://en.wikipedia.org/wiki/Speech_synthesis) engine,
which used [**International Phonetic Alphabet (IPA)**](
https://en.wikipedia.org/wiki/International_Phonetic_Alphabet) to pronounce vowels and consonants,
instead of normal alphabets.
It synthesised human voices and was intended to be programmed to learn from voices
and speak any kind of them.
In other words, it tried to imitate a **human child**!

As I learnt more and more about [Telecommunications](https://en.wikipedia.org/wiki/Telecommunications),
I preferred a more advanced method in Mergen III, see below.

#### Mergen III (v0.3)

The third version of Mergen, started at 29 March 2021 after many researches, had the same server-client model,
this time "temporarily" on a [**Local Area Network (LAN)**](https://en.wikipedia.org/wiki/Local_area_network),
instead of a server thousands of kilometres away!

Since my smartphone couldn't handle such heavy jobs and my laptop couldn't move,
I decided to do the heavy job of training/learning in a [**local PC server in Python**](
https://github.com/fulcrum6378/mergen_server) and alongside an [**Android client in Kotlin**](
https://github.com/fulcrum6378/mergen_client) whose...

1. Camera served as an **eye**.
2. Microphone served as **ears**.
3. Speaker served as a **mouth**.

Mergen I was going to be like an adult human being and Mergen II was a human child,
but I decided that I should start from something much, much earlier: **an animal**.
I decided to programme an animal, then gradually raise it to be perhaps someone more logical than a human being!

Gradually I concluded that this server-client model and these radio waves take such a huge amount of time to be
transferred and processed, and since I couldn't afford *robotic* resources, I was disappointed,
so I cancelled this one too after a year at 10 March 2022 :(

### Mergen IV (v0.4 - current version)

I decided to use C++ instead of Python and do all the job temporarily inside an Android smartphone,
so I began developing **Mergen IV** at 2 August 2022!

At 10 July 2023, I started a subproject for the computer vision (VIS) component and named it [**MyCV**](
https://github.com/fulcrum6378/mycv). It uses Python for faster debugging of the vision-related methods.
After its successful image processing, I started translating it to C++ since 24 September 2023.
Fortunately the C++ implementations worked in my phone, [*Samsung Galaxy A50*](
https://www.gsmarena.com/samsung_galaxy_a50-9554.php), **10 to 50 times faster** than Python(3.11) codes in my laptop
with a CPU of Intel Core i7!!

At 16 October 2023, I started another subproject, called it [**MergenLinux**](
https://github.com/fulcrum6378/mergen_linux), which runs Mergen on the Linux OS,
so that I could learn how to implement them in a more difficult and lesser known OS, such as *FreeBSD*.
But surprisingly, with few modifications it could run it on FreeBSD at 1 November!

At 3 November 2023, I started another subproject, this time for the audio processing/sound recognition (AUD) component
(in Python again like *MyCV*) and named it [Echo](https://github.com/fulcrum6378/echo).
Since Linux (Ubuntu) couldn't do the distance on my new decision for AUD to implement [Open Sound System](
https://en.wikipedia.org/wiki/Open_Sound_System), I archived *MergenLinux*,
and then forked it as [**Mergen4BSD**](https://github.com/fulcrum6378/mergen4bsd) at 12 November 2023.
At 28 December 2023, concluded that creating subprojects for AUD and HPT is useless;
because they don't need such pre-processing of their signals as VIS requires for its image frames!

After I realised that FreeBSD has so limited support for Vulkan, OpenCL and CUDA,
I was disappointed with FreeBSD and archived Mergen4BSD at 21 April 2024!
And decided to continue MergenLinux, but not so simultaneously.

At 30 April 2024, started another subproject called [**MyPR**](https://github.com/fulcrum6378/mypr)
for *Pattern Recognition*.

At 27 May 2024, decided to change Mergen's way from being used on a phone to
working in a 3D simulated environment, and decided to stop Mergen v0.4!

### Timeline of development

This table shows how much I've worked on Mergen and when. The timeline is divided into multiple consecutive periods.
(updated as of 6403.03.07+)

| # | Starting date          | Duration | Active days | %  |       Version        |
|---|:-----------------------|:---------|:-----------:|:--:|:--------------------:|
| 1 | 2020/12/13, 6399/09/23 | 43 days  |     20      | 47 | 0.1..0.2, pronouncer |
| 2 | 2021/01/25, 6399/11/06 | 264 days |     ~33     | 13 |         0.3          |
| 3 | 2021/10/16, 6400/07/24 | 145 days |     18      | 12 |         0.3          |
| 4 | 2022/03/10, 6400/12/19 | 137 days |      0      | 0  |          -           |
| 5 | 2022/07/26, 6401/05/04 | 109 days |     7+8     | 14 |         0.4          |
| 6 | 2022/11/12, 6401/08/21 | 126 days |    13+27    | 32 |         0.4          |
| 7 | 2023/03/18, 6401/12/27 | 42 days  |     0+1     | 2  |          -           |
| 8 | 2023/04/29, 6402/02/09 | 217 days |    88+13    | 47 | 0.4, mycv, linux+bsd |
| 9 | 2023/12/02, 6402/09/11 | 177 days |    64+1     | 37 | 0.4, bsd+linux, mypr |

- Periods 1, 2 and 3 include days of researches which are not recorded.
- Period 5 includes a few days of researching/practicing about/on a new programming language
  for Mergen (4 days), JNI+NDK (3 days) and OCR (1 day).
- Periods 6, 7 and 8 include a few days when I was learning/evaluating Vulkan (18-0-0 days),
  OpenBSD (2-0-0 days), FreeBSD (1-0-4 days), TensorFlow (3-0-0 days), PyTorch (3-1-0/5 days),
  Statistics (Data Science, 0-0-3/5 days) and Machine Learning (0-0-5 days).
- Period 9 includes 1 day for configuring *clspv*.

## Android app

[**Main.java**](android/java/ir/mahdiparastesh/mergen/Main.java) provides a basic control panel for this robot.

1. **Double-tap** anywhere **to start Mergen**.
2. Press the **Back button** of the phone **to stop Mergen**.

Once Mergen is started, it'll receive all kinds of visual, auditory and haptic inputs.
Ergo, touching the screen while Mergen is online is handled by [**HPT**](cpp/hpt) as a haptic input;
as if it senses your finger and processes your pattern of touching!
That's why I didn't use double-click to stop Mergen.

There is also a remote control / debugging tool, [**bridge.py**](android/bridge.py),
which communicates over a network (like Wi-Fi) as a client with
[Bridge.java](android/java/ir/mahdiparastesh/mergen/Bridge.java) which is the server.

The app **requires Android 10+**.
It uses Java 21 and the latest stable versions of Gradle, Android SDK, NDK and CMake.
No third-party dependencies needed at all as of now.

## License

```
Copyright © Mahdi Parastesh - USE IT WELL!!
```
