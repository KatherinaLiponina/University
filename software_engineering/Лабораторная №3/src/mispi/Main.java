package mispi;
import java.util.Scanner;

/** В классе Main подсчитывается количество вхождений первого введенного слова во всю строку **/
public class Main {

	public static void main(String[] args) {
	Scanner scanner = new Scanner(System.in);
        String substring = scanner.next();
        SearchedString search = new SearchedString(substring);
        String text = scanner.nextLine();
        int f = search.NumberOfEntry(text);
        System.out.println(f + 1);
        scanner.close();
	}

}
