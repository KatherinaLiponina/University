package mispi;
import javax.management.NotificationBroadcaster;

/*
Если не введенные слова не присутсвовали в тексте три раза подряд - посылается уведомление
*/
public interface MissedNotMBean extends NotificationBroadcaster {
	
	boolean isThreeInRow();
	int getMissedInRow();

}
