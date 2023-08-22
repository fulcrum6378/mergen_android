# Mergen

Mergen is a [free and open-source](https://en.wikipedia.org/wiki/Free_and_open-source_software)
logical real-time artificial-intelligence robot, in other words an "**Artificial Mind**",
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
   this section of Mergen is intended to be programmed using [**Machine Learning** (ML)](
   https://en.wikipedia.org/wiki/Machine_learning).
2. **God** aspect : this section which contains the robot's beliefs, is **NOT** intended to be programmed
   using *Machine Learning*, so it won't learn things from anybody, **unlike human beings**!
   Instead, it'll compute probabilities of any [Theorem](https://en.wikipedia.org/wiki/Theorem) and act
   in accordance with the [**Probabilist**](https://en.wikipedia.org/wiki/Probabilism) philosophy.
   So this robot CANNOT be brainwashed for dirty purposes (unless you hack or reprogramme it)!

> “To live alone one must be either a beast or a god, says Aristotle.
> Leaving out the third case: one must be both - a philosopher.”
> ― Nietzsche Friedrich Wilhelm

#### Dangers & Solutions

- Robots in films are shown to be stubbornly doing crazy things, but this trait shall never be acceptable in Mergen;
  He must be programmed to always be sceptical about himself, he must be really **Indecisive**!
  And consult everything with a human being.
- Using a [Reward System](https://en.wikipedia.org/wiki/Reward_system) in robots, namely [Reinforcement Learning](
  https://en.wikipedia.org/wiki/Reinforcement_learning) could also be dangerous, this is intended to be removed in
  Mergen in a near future.

## Parts

Each part has its own README in its own directory.

|      Codename      | Description                                            |
|:------------------:|:-------------------------------------------------------|
| [**AUD**](cpp/aud) | everything related to **Auditory** inputs and outputs. |
| [**HPT**](cpp/hpt) | everything related to **Touch** (haptic) inputs.       |
| [**MEM**](cpp/mem) | everything related to **Memory**.                      |
| [**MOV**](cpp/mov) | everything related to **Movements**.                   |
| [**VIS**](cpp/vis) | everything related to **Visual** inputs and outputs.   |

Deprecated parts:

- [REW](cpp/rew) : everything related to the *Reward System* (reinforcement learning).
- [VLK](cpp/vlk) : everything related to the *Vulkan* API (will be merged into the others).

## History

#### Theorem

At 21 August 2020, I wanted to programme an app based on [**logical theorems**](https://en.wikipedia.org/wiki/Theorem)
using which one can enter a sentence as a theorem and numerous chance factors below it which can make it True or False.
The app had to calculate their mathematical mean and say, for example:

> There is 20.4657% chance that **a god** exists!

This idea gradually turned into a robot who could do the exact same thing!

#### Mergen I (v0.1)

As I was getting more and more familiar with machine learning, I wanted to programme an Android app whose language was
merely Python using a specific library, but it had a weird license, so I forgot about using Python in Android.
Then I decided to use [ML Kit](https://developers.google.com/ml-kit), but it didn't satisfy me either!
I wanted to name the robot I had planned to create "Horus", but then I found a better name for it!

The first version of *Mergen*, started at **13 December 2020**, was intended to be a
[natural language processing (NLP)](https://en.wikipedia.org/wiki/Natural_language_processing) bot,
something like ChatGPT today.

It was intended to process **only digital texts** in a server using pure Python, and the server had to communicate with
an Android client app.
But I cancelled its development and pursued a better method in Mergen II, see below.

#### Mergen II (v0.2)

The second version of Mergen, started at 10-12 January 2021, was intended to primarily process audio,
I also called it the [**Pronouncer**](https://github.com/fulcrum6378/pronouncer).
It was a more advanced version of a [Text-To-Speech (TTS)](https://en.wikipedia.org/wiki/Speech_synthesis) engine,
which pronounced vowels and consonants of the [**International Phonetic Alphabet (IPA)**](
https://en.wikipedia.org/wiki/International_Phonetic_Alphabet) instead of normal alphabets.
It synthesised human voices and was intended to be programmed to learn from voices and speak any kind of them.

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

Gradually I concluded that this server-client model and these radio waves takes such a huge amount of time to
communicate, and since I couldn't afford *robotic* resources, I was disappointed,
so I cancelled this one too after a year at 10 March 2022 :(

### Mergen IV (v0.4 - current version)

I decided to use C++ instead of Python and do all the job temporarily inside an Android smartphone,
so I began developing **Mergen IV** at 2 August 2022!

At 10 July 2023, I also started a subproject for the computer vision part, called [**MyCV**](
https://github.com/fulcrum6378/mycv) using Python for faster debugging of the vision-related algorithms.

#### Timeline of Mergen's development periods

This table shows how much I've worked on Mergen and when.

| Starting date           | Duration | Active days | Score |  Version   |
|:------------------------|:---------|:-----------:|:-----:|:----------:|
| 2020/12/13 - 6399/09/23 | 43 days  |     20      |  47   |  0.1..0.2  |
| 2021/01/25 - 6399/11/06 | 264 days |     ~33     |  13   |  0.2..0.3  |
| 2021/10/16 - 6400/07/24 | 145 days |     18      |  12   |    0.3     |
| 2022/03/10 - 6400/12/19 | 144 days |      0      |   0   |     -      |
| 2022/08/02 - 6401/05/11 | 102 days |      9      |   9   |    0.4     |
| 2022/11/12 - 6401/08/21 | 126 days |     43      |  34   |    0.4     |
| 2023/03/18 - 6401/12/27 | 42 days  |      1      |   2   |     -      |
| 2023/04/29 - 6402/02/09 | ongoing  |     28      |  24   | 0.4 (+0.3) |

## License

```
VIM LICENSE

I)  There are no restrictions on distributing unmodified copies of Mergen
    except that they must include this license text.  You can also distribute
    unmodified parts of Mergen, likewise unrestricted except that they must
    include this license text.  You are also allowed to include executables
    that you made from the unmodified Mergen sources, plus your own usage
    examples.

II) It is allowed to distribute a modified (or extended) version of Mergen,
    including executables and/or source code, when the following four
    conditions are met:
    1) This license text must be included unmodified.
    2) The modified Mergen must be distributed in one of the following five
       ways:
       a) If you make changes to Mergen yourself, you must clearly describe
          in the distribution how to contact you.  When the maintainer asks
          you (in any way) for a copy of the modified Mergen you
          distributed, you must make your changes, including source code,
          available to the maintainer without fee.  The maintainer reserves
          the right to include your changes in the official version of
          Mergen.  What the maintainer will do with your changes and under
          what license they will be distributed is negotiable.  If there has
          been no negotiation then this license, or a later version, also
          applies to your changes. The current maintainer is Mahdi Parastesh
          <fulcrum1378@gmail.com>.  If this changes it will be announced in
          appropriate places (most likely mahdiparastesh.ir).  When it is
          completely impossible to contact the maintainer, the obligation to
          send him your changes ceases.  Once the maintainer has confirmed
          that he has received your changes they will not have to be sent
          again.
       b) If you have received a modified Mergen that was distributed as
          mentioned under a) you are allowed to further distribute it
          unmodified, as mentioned at I).  If you make additional changes the
          text under a) applies to those changes.
       c) Provide all the changes, including source code, with every copy of
          the modified Mergen you distribute.  This may be done in the form
          of a context diff.  You can choose what license to use for new code
          you add.  The changes and their license must not restrict others
          from making their own changes to the official version of Mergen.
       d) When you have a modified Mergen which includes changes as
          mentioned under c), you can distribute it without the source code
          for the changes if the following three conditions are met:
          - The license that applies to the changes permits you to distribute
            the changes to the Mergen maintainer without fee or restriction,
            and permits the Mergen maintainer to include the changes in the
            official version of Mergen without fee or restriction.
          - You keep the changes for at least three years after last
            distributing the corresponding modified Mergen.  When the
            maintainer or someone who you distributed the modified Mergen
            to asks you (in any way) for the changes within this period, you
            must make them available to him.
          - You clearly describe in the distribution how to contact you.  This
            contact information must remain valid for at least three years
            after last distributing the corresponding modified Mergen, or
            as long as possible.
       e) When the GNU General Public License (GPL) applies to the changes,
          you can distribute the modified Mergen under the GNU GPL version
          2 or any later version.
    3) A message must be added, at least in the intro screen, such that the
       user of the modified Mergen is able to see that it was modified.
       When distributing as mentioned under 2)e) adding the message is only
       required for as far as this does not conflict with the license used
       for the changes.
    4) The contact information as required under 2)a) and 2)d) must not be
       removed or changed, except that the person himself can make
       corrections.

III) If you distribute a modified version of Mergen, you are encouraged to
     use the Vim license for your changes and make them available to the
     maintainer, including the source code.  The preferred way to do this is
     by e-mail or by uploading the files to a server and e-mailing the URL. If
     the number of changes is small (e.g., a modified Makefile) e-mailing a
     context diff will do.  The e-mail address to be used is
     <fulcrum1378@gmail.com>

IV)  It is not allowed to remove this license from the distribution of the
     Mergen sources, parts of it or from a modified version.  You may use
     this license for previous Mergen releases instead of the license that
     they came with, at your option.

```
