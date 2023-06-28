#ifndef MOV_VIBRATOR_H
#define MOV_VIBRATOR_H

#define OUTPUT_ID_VIBRATOR -3

class Vibrator {
public:
    Vibrator();

    static void OnReward(double fortuna);

    /*jlong testShake = 200;
    jmethodID shakeMethod = env->GetMethodID(
            env->FindClass("ir/mahdiparastesh/mergen/Main"), "shake", "(J)V");
    env->CallVoidMethod(main, shakeMethod, testShake);*/

    ~Vibrator();
};

#endif //MOV_VIBRATOR_H
