
import static org.junit.Assert.*;
import org.junit.*;
import mispi.SearchedString;

/** TestSearchedString содержит 9 тестов для проверки работы строки с префикс-функцией**/
public class TestSearchedString {

private SearchedString substring;

	@Before
	public void create() {
		substring = new SearchedString("exe");
	}
	@After
	public void uncreate() {
		substring = null;
	}
	
	/** atStart() проверяет работу поиска подстроки в начале текста **/
	@Test
	public void atStart() {
		String text = "exemple";
		assertTrue(substring.FirstEntry(text) == 0);
	}
	
	/** inTheMiddle() проверяет работу поиска подстроки в середине текста **/
	@Test
	public void inTheMiddle() {
		String text = "planexexpect";
		assertTrue(substring.FirstEntry(text) == 4);
	}
	
	/** inTheEnd() проверяет работу поиска подстроки в конце текста **/
	@Test
	public void inTheEnd() {
		String text = "planexe";
		assertTrue(substring.FirstEntry(text) == 4);
	}
	
	/** twoEntry() проверяет работу поиска подстроки с двумя вхождениями **/
	@Test
	public void twoEntry() {
		String text = "eexeabexeal";
		assertTrue(substring.NumberOfEntry(text) == 2);
	}
	
	/** threeEntry() проверяет работу поиска подстроки с тремя вхождениями **/
	@Test
	public void threeEntry() {
		String text = "eexeabexeaexel";
		assertTrue(substring.NumberOfEntry(text) == 3);
	}
	
	/** twoCrossed() проверяет работу поиска подстроки с пересекающимися вхождениями подстроки **/
	@Test
	public void twoCrossed() {
		SearchedString tmp = new SearchedString("aaa");
		String text = "awayaaaaway";
		assertTrue(tmp.NumberOfEntry(text) == 2);
	}
	
	/** noEntry() проверяет работу поиска подстроки в тексте без вхождений **/
	@Test
	public void noEntry() {
		String text = "exaexagfdex";
		assertTrue(substring.NumberOfEntry(text) == 0);
	}
	
	/** secondEntry() проверяет работу поиска для второго вхождения **/
	@Test
	public void secondEntry() {
		String text = "fexeawexey";
		int position = substring.FirstEntry(text);
		position = substring.FirstEntry(text, position + 1);
		assertTrue(position == 6);
	}
	
	/** globalSearch() проверяет работу поиска подстроки в стихотворении **/
	@Test(timeout=5)
	public void globalSearch() {
		substring = new SearchedString("fire");
		String text = "Some say the world will end in fire,\n" + 
				"Some say in ice.\n" + 
				"From what I’ve tasted of desire\n" + 
				"I hold with those who favor fire.\n" + 
				"But if it had to perish twice,\n" + 
				"I think I know enough of hate\n" + 
				"To say that for destruction ice\n" + 
				"Is also great\n" + 
				"And would suffice.";
		assertTrue(substring.NumberOfEntry(text) == 2);
	}

}

