#ifndef PROMPTS_H
#define PROMPTS_H

static const char* PROMPTS[] = {
    "Sun",
    "Apple",
    "House",
    "Cat",
    "Dog",
    "Car",
    "Flower",
    "Tree",
    "Fish",
    "Bird",
    "Mountain",
    "Cloud",
    "Star",
    "Moon",
    "Umbrella",
    "Book",
    "Chair",
    "Cup",
    "Clock",
    "Heart",
    "Boat",
    "Plane",
    "Bicycle",
    "Ball",
    "Butterfly"
};

#define PROMPT_COUNT (sizeof(PROMPTS) / sizeof(PROMPTS[0]))

typedef struct {
    int min_score;
    const char* title;
    const char* description;
} Title;

static const Title TITLES[] = {
    {10, "MASTER ARTIST", "You are a true artistic genius!"},
    {7, "RISING STAR", "Your talent is blossoming!"},
    {5, "GETTING GOOD", "Nice artistic skills!"},
    {3, "AMATEUR", "Keep practicing!"},
    {1, "BEGINNER", "You need more practice..."},
    {0, "NO TALENT", "Maybe try something else..."}
};

#define TITLE_COUNT (sizeof(TITLES) / sizeof(TITLES[0]))

static inline const Title* get_title(int score) {
    for (int i = 0; i < (int)TITLE_COUNT; i++) {
        if (score >= TITLES[i].min_score) {
            return &TITLES[i];
        }
    }
    return &TITLES[TITLE_COUNT - 1];
}

#endif // PROMPTS_H
