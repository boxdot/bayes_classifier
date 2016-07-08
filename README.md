## Build

Put [rapidjson](https://github.com/miloyip/rapidjson) source into `$HOME/src` and build with

```
make
```

## Classify

The train and test data are available at https://www.kaggle.com/c/whats-cooking. Classify with:

```
./bayes_classifier train.json test.json > submission.csv
```
