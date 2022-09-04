package mispi;

/** Класс SearchedString содержит строку и префикс-функцию для нее **/
public class SearchedString {
	private final String Substring;
    private int [] PrefixFunction;

    /** Функция ComputePrefixFunction вызывается конструктором для расчета префиксной функции **/
    private void ComputePrefixFunction() {
        int n = Substring.length();
        PrefixFunction = new int [n];
        PrefixFunction[0] = 0;
        int k = 0;
        for (int q = 1; q < n; q++) {
            while (k > 0 && Substring.charAt(k) != Substring.charAt(q)) {
                k = PrefixFunction[k - 1];
            }
            if (Substring.charAt(k) == Substring.charAt(q)) {
                k++;
            }
            PrefixFunction[q] = k;
        }
    }

    public SearchedString(String substing) {
        Substring = substing;
        ComputePrefixFunction();
    }

    public String substring() {
        return Substring;
    }

    /** Функция FirstEntry(String text, int position) находит первое вхождение подстроки, начиная с некоторой позиции, задаваемой вторым аргументом **/
    public int FirstEntry(String text, int position) {
        if (position < 0) return -1;
        int n = text.length();
        int m = this.Substring.length();
        int q = 0;
        for (int i = position; i < n; i++) {
            while (q > 0 && this.Substring.charAt(q) != text.charAt(i)) {
                q = this.PrefixFunction[q - 1];
            }
            if (this.Substring.charAt(q) == text.charAt(i)) {
                q++;
            }
            if (q == m) {
                return i - m + 1;
            }
        }
        return -1;
    }
    /** Функция FirstEntry(String text) вызывает FirstEntry(String text, int position) с position = 0 **/
    public int FirstEntry(String text) {
        return FirstEntry(text, 0);
    }

    /** Функция NumberOfEntry считает количество входов подстроки при помощи функции FirstEntry **/
    public int NumberOfEntry(String text) {
        int pos = -1, number = 0;
        do {
            pos = FirstEntry(text, pos + 1);
            number++;
        } while (pos != -1);
        return number - 1;
    }

}

