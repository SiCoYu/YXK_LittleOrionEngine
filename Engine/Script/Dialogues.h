#ifndef  __DIALOGUES_H__
#define  __DIALOGUES_H__

#include <string>
#include <vector>


enum class DialogueID
{
	INTRO = 0,
	JUMPANDDASH = 1,
	HOWTOFIGHT = 2,
	LIGHTBALL = 3,
	LIGHTORBS = 4,
	ALLY_INTRO = 5,
	ALLY_INTRO_REPEAT = 6,
	ALLY_INTRO_TUTORIAL_SHOWED = 7,
	DO_NOT_IGNORE = 8,
	ALLY_CORRUPTED_WATER = 9,
	ALLY_LEVEL1_CELEBRATION = 10,
	LIGHTSHIELD_TUTORIAL_START = 12,
	LIGHTSHIELD_TUTORIAL_REPEAT = 13,
	LIGHTSHIELD_TUTORIAL_END = 14,
	DO_NOT_IGNORE2 = 11,
};

enum class DialogueFeeling
{
	FINISH = -1,
	IDLE = 0,
	SAD = 1,
	HAPPY = 2
};

struct Sentence
{
	std::string text;
	DialogueFeeling feeling;
};

struct Dialogue
{
	DialogueID id;
	std::vector<Sentence> sentences;
	virtual void Init() = 0;
};

struct Intro : Dialogue
{
	void Init() override
	{
		id = DialogueID::INTRO;
		sentences.push_back(Sentence{ "Hello Darkness my old friend" });
	}
};

struct AllyIntro : Dialogue
{
	void Init() override
	{
		id = DialogueID::ALLY_INTRO;
		sentences.push_back(Sentence{ "Hey, Skogfeer! We need your help!", DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "Niberry Forest has fallen into a state  of corruption.", DialogueFeeling::SAD });
		sentences.push_back(Sentence{ "You are our only hope to save it!", DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "I saw some ancient runes around the      forest.", DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "I am sure they are important, but I've never been good with glyphs..." , DialogueFeeling::SAD });
		sentences.push_back(Sentence{ "Check them out!", DialogueFeeling::HAPPY });
		sentences.push_back(Sentence{ "They might show us the way forward.", DialogueFeeling::HAPPY });
		sentences.push_back(Sentence{ "Perhaps even secrets for healing the       forest!", DialogueFeeling::HAPPY });

	}
};

struct AllyIntroRepeat : Dialogue
{
	void Init() override
	{
		id = DialogueID::ALLY_INTRO_REPEAT;
		sentences.push_back(Sentence{ "Take a look at the ancient runes and        return!",DialogueFeeling::IDLE });
	}
};

struct AllyPopupShowed: Dialogue
{
	void Init() override
	{
		id = DialogueID::ALLY_INTRO_TUTORIAL_SHOWED;
		sentences.push_back(Sentence{ "You can read the runes! Amazing." ,DialogueFeeling::HAPPY});
		sentences.push_back(Sentence{ "We need to go deeper into the forest. I'll lead the way!",DialogueFeeling::HAPPY });
		sentences.push_back(Sentence{ "See ya!",DialogueFeeling::IDLE });
	}
};

struct AllyLightBallTransform : Dialogue
{
	void Init() override
	{
		id = DialogueID::LIGHTBALL;
		sentences.push_back(Sentence{ "Great job!" ,DialogueFeeling::HAPPY });
		sentences.push_back(Sentence{ "The Mushdooms fear your light!" ,DialogueFeeling::HAPPY });
		sentences.push_back(Sentence{ "There may still be hope for Nibbery..." ,DialogueFeeling::HAPPY });
		sentences.push_back(Sentence{ "It's dangerous for me to go alone..." ,DialogueFeeling::SAD });
		sentences.push_back(Sentence{ "I'll come with you! Trust me, I'll come  in handy!" ,DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "Try to throw me!" ,DialogueFeeling::IDLE });
	}
};

struct AllyDoNotIgnoreMe : Dialogue
{
	void Init() override
	{
		id = DialogueID::DO_NOT_IGNORE;
		sentences.push_back(Sentence{ "Hey! Don't ignore me!",DialogueFeeling::SAD });
	}
};

struct AllyDoNotIgnoreMe2 : Dialogue
{
	void Init() override
	{
		id = DialogueID::DO_NOT_IGNORE2;
		sentences.push_back(Sentence{ "Help me, please!",DialogueFeeling::SAD });
	}
};
struct AllyCorruptedWater : Dialogue
{
	void Init() override
	{
		id = DialogueID::ALLY_CORRUPTED_WATER;
		sentences.push_back(Sentence{ "Wait! That water looks corrupted!",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "Try to jump over it!",DialogueFeeling::IDLE });
	}
};

struct AllyLevel1Celebration : Dialogue
{
	void Init() override
	{
		id = DialogueID::ALLY_LEVEL1_CELEBRATION;
		sentences.push_back(Sentence{ "Nice Job!",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "We make a great team!",DialogueFeeling::IDLE });
	}
};

struct AllyLightShieldTutorial : Dialogue
{
	void Init() override
	{
		id = DialogueID::LIGHTSHIELD_TUTORIAL_START;
		sentences.push_back(Sentence{ "We did a great job!",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "But we can do it better.",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "I will teach you something cool!",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "You can create a shield using your       light.",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "I will bounce against that shield.",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "Let's try it out!",DialogueFeeling::IDLE });
	}
};

struct AllyLightShieldTutorialEnd : Dialogue
{
	void Init() override
	{
		id = DialogueID::LIGHTSHIELD_TUTORIAL_END;
		sentences.push_back(Sentence{ "Nice job! You can make me bounce      many times",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "Use the shield to protect yourself too!",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "But be careful!",DialogueFeeling::IDLE });
		sentences.push_back(Sentence{ "If it breaks you will be stunned.",DialogueFeeling::IDLE });
	}
};

struct AllyLightShieldTutorialRepeat : Dialogue
{
	void Init() override
	{
		id = DialogueID::LIGHTSHIELD_TUTORIAL_REPEAT;
		sentences.push_back(Sentence{ "Throw me and then use the shield     when I am about to reach you.",DialogueFeeling::IDLE });
	}
};
#endif

