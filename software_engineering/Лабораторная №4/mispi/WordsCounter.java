package mispi;

public class WordsCounter implements WordsCounterMBean {
	private int wordsNumber = 0;
	private int missedNumber = 0;

	public void updateNumber(boolean success) {
		wordsNumber++;
		if (!success) missedNumber++; 
	}
	
	@Override
	public double SuccessPct() {
		return (1 - (double)missedNumber / wordsNumber);
	}

	@Override
	public int getWordsNumber() {
		return wordsNumber;
	}
	@Override
	public int getMissedNumber() {
		return missedNumber;
	}

}
