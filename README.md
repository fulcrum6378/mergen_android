# Mergen

Mergen is a [free and open-source](https://en.wikipedia.org/wiki/Free_and_open-source_software)
logical real-time robot, in other words an "**Artificial Mind**",
written in C++ language, which is temporarily mounted on Android.

The main C++ library located in the [/cpp/](cpp) is intended to be mounted on real robots with the
[*FreeBSD* operating system](https://www.freebsd.org/).

## Philosophy

Mergen is intended to become a robot that can think more logically and wisely than human beings,
and help them have a better life!

Mergen shall differ human beings in the way of thinking, so that it won't trust anybody's words;
it just seeks truths and to improve conditions for all living beings!

> Mergen was named after a [*Turkic* god](https://en.wikipedia.org/wiki/Mergen) of abundance and **wisdom**!

It is intended to serve as something like an operating system for AI robots, its job is to
process vision, audio and touch and act logically based on them and according to its purpose,
define by its Reward System (REW)!

Mergen is intended to have 2 aspects:

1. **Animal** aspect : similar to the [**Unconscious mind**](https://en.wikipedia.org/wiki/Unconscious_mind)
   this section of Mergen is intended to be programmed to process raw image and audio inputs and touch patterns.
2. **God** aspect : this section which contains the robot's thinking system and beliefs, will compute probabilities
   of any logical [Theorem](https://en.wikipedia.org/wiki/Theorem) and act in accordance with the [**Probabilist**](
   https://en.wikipedia.org/wiki/Probabilism) philosophy.
   So this robot CANNOT be brainwashed for dirty purposes (unless you hack or reprogramme it)!

> “To live alone one must be either a beast or a god, says Aristotle.
> Leaving out the third case: one must be both - a philosopher.”
> ― Friedrich Wilhelm Nietzsche

#### Dangers & Solutions

- Robots in films are shown to be stubbornly doing crazy things, but this trait shall never be acceptable in Mergen;
  He must be programmed to always be sceptical about himself, he must be really **Indecisive**!
  And consult everything with a human being.

## Parts

|      Codename      | Description                                 |
|:------------------:|:--------------------------------------------|
| [**AUD**](cpp/aud) | controlling **Auditory** inputs and outputs |
| [**HPT**](cpp/hpt) | controlling **Touch** (haptic) inputs       |
| [**MOV**](cpp/mov) | controlling **Movements**                   |
| [**REW**](cpp/rew) | **Reward System** (reinforcement learning)  |
| [**SCM**](cpp/scm) | Simulation of the **Subconscious Mind**     |
| [**VIS**](cpp/vis) | controlling **Visual** inputs and outputs   |

Deprecated parts:

- [VLK](cpp/vlk) : contains two practices of the *Vulkan* API.

## History

#### Theorem

At 21 August 2020, I wanted to programme an app based on [**logical theorems**](https://en.wikipedia.org/wiki/Theorem)
using which one can enter a sentence as a theorem and numerous chance factors below it which can make it True or False.
The app had to calculate their mathematical mean and say, for example:

> There is 20.4657% chance that **a god** exists!

This idea gradually turned into a robot who could do the exact same thing!

#### Mergen I (v0.1)

As I was getting more and more familiar with the machine learning concepts, I wanted to programme an Android app whose
programming language was merely Python using Chaquopy, but since it had a weird bothersome license,
I decided to forget about using Python in Android. Then I decided to try [ML Kit](
https://developers.google.com/ml-kit), but it had no use to me either!
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
It was a more advanced version of a [Text-To-Speech (TTS)](https://en.wikipedia.org/wiki/Speech_synthesis) engine,
which pronounced vowels and consonants of the [**International Phonetic Alphabet (IPA)**](
https://en.wikipedia.org/wiki/International_Phonetic_Alphabet) instead of normal alphabets.
It synthesised human voices and was intended to be programmed to learn from voices and speak any kind of them.
In other words, it tried to imitate a **human child**!

As I learnt more and more about [Telecommunications](https://en.wikipedia.org/wiki/Telecommunications),
I preferred a better method in Mergen III, see below.

#### Mergen III (v0.3)

The third version of Mergen, started at 29 March 2021 after many researches, had the same server-client approach,
this time *temporarily* on a [**Local Area Network (LAN)**](https://en.wikipedia.org/wiki/Local_area_network),
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
I decided to programme an animal then gradually grow it to be a super-human being, namely a god of logic!!!

Gradually I concluded that this server-client model and these radio waves takes such a huge amount of time to
communicate, and since I couldn't afford *robotic* resources, I was disappointed,
so I cancelled this one too after a year at 10 March 2022 :(

### Mergen IV (v0.4 - current version)

I decided to use C++ instead of Python and do all the job temporarily inside an Android smartphone,
so I began developing **Mergen IV** at 2 August 2022!

In 10 July 2023, I started a subproject for the computer vision (VIS) part and named it [**MyCV**](
https://github.com/fulcrum6378/mycv).It uses Python for faster debugging of the vision-related methods.
After its successful image processing, I started translating it to C++ since 24 September 2023.

## Timeline of development

This table shows how much I've worked on Mergen and when. The timeline is divided into multiple consecutive periods.
(updated as of 2023.10.02)

| # | Starting date          | Duration | Active days | %  |   Version   |
|---|:-----------------------|:---------|:-----------:|:--:|:-----------:|
| 1 | 2020/12/13, 6399/09/23 | 43 days  |     20      | 47 |  0.1..0.2   |
| 2 | 2021/01/25, 6399/11/06 | 264 days |     ~33     | 13 |  0.2..0.3   |
| 3 | 2021/10/16, 6400/07/24 | 145 days |     18      | 12 |     0.3     |
| 4 | 2022/03/10, 6400/12/19 | 137 days |      0      | 0  |      -      |
| 5 | 2022/07/26, 6401/05/04 | 109 days |     7+8     | 14 |     0.4     |
| 6 | 2022/11/12, 6401/08/21 | 126 days |    13+27    | 32 |     0.4     |
| 7 | 2023/03/18, 6401/12/27 | 42 days  |     0+1     | 2  |      -      |
| 8 | 2023/04/29, 6402/02/09 | 156 days |    44+11    | 35 | 0.4 (+mycv) |

- Periods 1, 2 and 3 include days of researches which are not recorded.
- Period 5 includes a few days of researching/practicing about/on a new programming language
  for Mergen (4 days), JNI+NDK (3 days) and OCR (1 day).
- Periods 6, 7 and 8 include a few days when I was learning/evaluating Vulkan (18-0-0 days),
  OpenBSD (2-0-0 days), FreeBSD (1-0-2 days), TensorFlow (3-0-0 days), PyTorch (3-1-0/5 days),
  Statistics (Data Science, 0-0-3/5 days) and Machine Learning (0-0-5 days).

## License

```
Copyright © Mahdi Parastesh - All Rights Reserved.
```
