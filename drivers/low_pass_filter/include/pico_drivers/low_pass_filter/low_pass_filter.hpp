/**
 * @file low_pass_filter.hpp
 * @brief this module defines a software low pass filter
 */

#ifndef _LOW_PASS_FILTER_H_
#define _LOW_PASS_FILTER_H_

class LowPassFilter {
   public:

    /**
     * @brief default constructor
     */
    LowPassFilter();

    /**
     * @brief Constructor with defaults
     *
     * @param cut_off_freq filters cut off frequency
     * @param delta_time time between updates in seconds
     */
    LowPassFilter(float cut_off_freq, float delta_time);

    /**
     * @brief
     *
     * @param input
     * @return
     */
    float update(float input);

    /**
     * @brief
     *
     * @param input
     * @param delta_time time between updates in seconds
     * @param cut_off_freq filters cut off frequency
     * @return
     */
    float update(float input, float delta_time, float cut_off_freq);

    /**
     * @brief
     *
     * @param delta_time time between updates in seconds
     * @param cut_off_freq filters cut off frequency
     */
    void reconfigure_filter(float delta_time, float cut_off_freq);

    /**
     * @brief
     *
     * @return
     */
    float get_output() const;

   private:
    float output;
    float ePow;
};

#endif //end _LOW_PASS_FILTER_H_
