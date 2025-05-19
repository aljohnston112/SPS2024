A multi-threaded stock data analyzer written in C++. It uses a modified version of csvmonkey that makes it faster. It transforms daily pricing information into price direction data and uses a probabilistic approach attempting to predict future price direction data. A few approaches were taken. 

The first involved a probability distribution on price direction information and used a single time point from the past to try and predict the future.

The second tried to use all binary combinations of price direction information as buy and sell indicators.

The third involved a tree that contained sequences of price direction data over time, so more points from the future could be used for prediction. 

All three attempts did not do well at making money.

Lastly, a frequency analysis was done using PyWavelets but the results did not look helpful.
