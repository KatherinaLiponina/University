package mispi;

import javax.management.Notification;
import javax.management.NotificationBroadcasterSupport;
import javax.management.AttributeChangeNotification;
import javax.management.MBeanNotificationInfo;

public class MissedNot extends NotificationBroadcasterSupport implements MissedNotMBean {
	
	private boolean ThreeInRow = false;
	private int InRow = 0;
	private int sequenceNumber = 0;

	public void updateCounter(boolean success) {
		if (success) {
			InRow = 0;
			ThreeInRow = false;
		}
		else {
			InRow++;
			if (InRow >= 3) {
				ThreeInRow = true;
			}
		}

			Notification notification = new Notification("threeAbsendInRow", this, ++sequenceNumber, System.currentTimeMillis(), "Three absent words in a row happend");
			sendNotification(notification);
	}

	@Override
	public boolean isThreeInRow() {
		return ThreeInRow;
	}
	@Override
	public int getMissedInRow() {
		return InRow;
	}

	@Override
      	public MBeanNotificationInfo[] getNotificationInfo(){
          	String[] types = new String[]{ AttributeChangeNotification.ATTRIBUTE_CHANGE};
          	String name = AttributeChangeNotification.class.getName();
          	String description = "Three absent words in a row happend";
		MBeanNotificationInfo info = new MBeanNotificationInfo(types, name, description);
		return new MBeanNotificationInfo[]{info};
	}

}
