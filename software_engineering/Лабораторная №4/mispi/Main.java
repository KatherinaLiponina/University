package mispi;
import java.util.Scanner;
import java.io.File;
import java.io.IOException;

import javax.management.*;
import java.lang.management.ManagementFactory;
import java.util.*;

/** В классе Main производится поиск слов в заданном заранее тексте **/
public class Main {

	public static void main(String[] args) throws MalformedObjectNameException, InstanceAlreadyExistsException, MBeanRegistrationException, NotCompliantMBeanException, IOException {

		//MBean
		MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
		WordsCounter wordsBean = new WordsCounter();
		ObjectName nameW = new ObjectName("mispi.Main:name=WordsCounter");
		mbs.registerMBean(wordsBean, nameW);
		MissedNot missBean = new MissedNot();
		ObjectName nameM = new ObjectName("mispi.Main:name=MissedNot");
		mbs.registerMBean(missBean, nameM);

		//search
		int entry_number = 0;
		Scanner word_scanner = new Scanner(System.in);
		File document = new File("/home/master/test/mispi_lab4/poem.txt");
		Scanner doc_scanner = new Scanner(document);

		while(word_scanner.hasNext()) {
			SearchedString substring = new SearchedString(word_scanner.next());
			int number = 0;
			while (doc_scanner.hasNextLine()) {
				String searchIn = doc_scanner.nextLine();
				int result = substring.NumberOfEntry(searchIn);
				number += result;
			}
			doc_scanner.close();
			doc_scanner = new Scanner(document);
			System.out.println(number);
			entry_number += number;
			
			wordsBean.updateNumber(number != 0);
			missBean.updateCounter(number != 0);
			
		}
		word_scanner.close();
		doc_scanner.close();	
	}

}
