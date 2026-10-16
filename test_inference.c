
#include <stdio.h>
#include <string.h>

int infer_drl_action(float queue, float temp, float core);

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--batch") == 0) {
        float queue, temp, core;

        while (scanf("%f %f %f", &queue, &temp, &core) == 3) {
            printf("%d\n", infer_drl_action(queue, temp, core));
        }

        return 0;
    }

    const float observations[5][3] = {
        {0.0f, 0.0f, 0.0f},
        {0.2f, 0.3f, 0.0f},
        {0.5f, 0.5f, 1.0f},
        {0.9f, 0.8f, 1.0f},
        {1.0f, 1.0f, 1.0f}
    };

    const int expected[5] = {0, 1, 1, 1, 1};
    int failures = 0;

    for (int i = 0; i < 5; i++) {
        int actual = infer_drl_action(
            observations[i][0],
            observations[i][1],
            observations[i][2]
        );

        printf("Test %d: expected=%d actual=%d %s\n",
               i + 1, expected[i], actual,
               actual == expected[i] ? "PASS" : "FAIL");

        if (actual != expected[i]) {
            failures++;
        }
    }

    if (failures) {
        printf("FAIL: %d inference mismatches\n", failures);
        return 1;
    }

    puts("PASS: C inference matches Python reference actions");
    return 0;
}
