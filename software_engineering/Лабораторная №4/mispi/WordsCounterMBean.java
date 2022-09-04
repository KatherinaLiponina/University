package mispi;

/*
Подсчет количества введенных слов, а так же числа ненайденных слов и возврат процента успеха.
*/

public interface WordsCounterMBean {
	
	double SuccessPct();
	int getWordsNumber();
	int getMissedNumber();

}
