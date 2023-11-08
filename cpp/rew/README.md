## REW: Reward System

All conscious actions made by animals, including human beings, are based on their
[*Reward System*](https://en.wikipedia.org/wiki/Reward_system).
In other words, all they do is to **seek pleasure** and **avoid pain**.
This fact is stated by the [Pleasure Principle](https://en.wikipedia.org/wiki/Pleasure_principle_(psychology))
and earlier by the [Hedonist philosophy](https://en.wikipedia.org/wiki/Hedonism).

**Pleasure and Pain** are mysterious phenomena present in most multicellular organisms.
The true nature of pleasure and pain is not discovered yet by the current science, that's why we can't implement it
on *robots*. But we still can simulate the process and create a fake reward system for a robot, which is only a number;
no emotion is felt by that robot!
Such artificial intelligence may be called a [*Philosophical Zombie*](https://en.wikipedia.org/wiki/Philosophical_zombie)
and the method used in it is called [**Reinforcement Learning**](https://en.wikipedia.org/wiki/Reinforcement_learning).

*Reinforcement Learning* might potentially be dangerous when implemented in robots!
That robot might burst out of control and
**do anything possible in order to achieve its FAKE pleasure and avoid its FAKE pain**!!!
That is why I am reluctant to implement it in my ultimate robot, but I implement a simple structure of it,
**ONLY as a practice** for manipulating senses and somehow *machine learning*.

This component contains the following modules:

- [**Rewarder**](rewarder.hpp) : the main module which contains the **Fortuna** value;
  a *double* value whose negativity means an overall pain and whose positivity means an overall pleasure,
  computed using [weighted arithmetic mean](https://en.wikipedia.org/wiki/Weighted_arithmetic_mean) from multiple other
  fortuna scores related to some CRITERIA.
  (the idea comes from my Android app related to pleasure and pain: [Fortuna](https://github.com/fulcrum6378/fortuna)
  and the names of the both come from a medieval song called [**O Fortuna**](https://en.wikipedia.org/wiki/O_Fortuna))

- [**Criterion**](criterion.hpp) : base class for criteria used for calculation of the ultimate Fortuna score.

- [**Expression**](expression.hpp) : base class for ways of expressing pleasure and pain;
  namely a base class for **emotions**.
