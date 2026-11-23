// Only drives controls in the app launched by the disposable Windows CI job.
using System;
using System.Text;
using System.IO;
using System.Security.Principal;
using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;
public static class CzWindow {
 [DllImport("user32",CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string cls,string title);
 [DllImport("user32")] public static extern uint GetWindowThreadProcessId(IntPtr w,out uint id);
 [DllImport("user32")] public static extern IntPtr GetDlgItem(IntPtr w,int id);
 [DllImport("user32")] public static extern IntPtr GetLastActivePopup(IntPtr w);
 [DllImport("user32")] public static extern bool IsWindowEnabled(IntPtr w);
 [DllImport("user32",EntryPoint="SendMessageW")] public static extern IntPtr Send(IntPtr w,int m,IntPtr a,IntPtr b);
 [DllImport("user32",EntryPoint="SendMessageW",CharSet=CharSet.Unicode)] static extern IntPtr PutText(IntPtr w,int m,IntPtr a,string b);
 [DllImport("user32",EntryPoint="SendMessageW",CharSet=CharSet.Unicode)] static extern IntPtr ReadText(IntPtr w,int m,IntPtr a,StringBuilder b);
 [DllImport("user32",EntryPoint="GetClassLongPtrW")] public static extern IntPtr ClassLong(IntPtr w,int id);
 [DllImport("user32",EntryPoint="PostMessageW")] static extern bool Post(IntPtr w,int msg,IntPtr a,IntPtr b);
 [DllImport("advapi32",CharSet=CharSet.Unicode,SetLastError=true)]
 static extern bool LogonUser(string user,string domain,string password,int type,int provider,out SafeAccessTokenHandle token);
 [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode)]
 struct Resource { public int scope,type,display,usage;public string local,remote,comment,provider; }
 [DllImport("mpr",CharSet=CharSet.Unicode)] static extern int WNetAddConnection2(ref Resource r,string password,string user,int flags);
 [DllImport("mpr",CharSet=CharSet.Unicode)] static extern int WNetCancelConnection2(string name,int flags,bool force);
 // First authenticate a distinct, non-persistent SMB session; on the admin thread
 // verify the identity Windows reports for a live open handle before trying a write.
 public static void TestShare(string user,string pass,string share,string marker,bool writable,Action<string> serverIdentity){
   if(!System.Text.RegularExpressions.Regex.IsMatch(share,"^CZtest[a-f0-9]{9}$"))throw new ArgumentException("not a fixture share");
   string host=Environment.MachineName,account=host+@"\"+user,root=@"\\127.0.0.1\"+share;
   SafeAccessTokenHandle tok;
   if(!LogonUser(user,host,pass,9,3,out tok))throw new Exception("Cannot prepare local test identity");
   using(tok){
     bool connected=false;FileStream held=null;
     try{
       held=WindowsIdentity.RunImpersonated(tok,delegate{
         var net=new Resource{type=1,remote=root};int rc=WNetAddConnection2(ref net,pass,account,4);
         if(rc!=0)throw new Exception("Cannot establish explicit fixture SMB session: "+rc);connected=true;
         return new FileStream(Path.Combine(root,"check.txt"),FileMode.Open,FileAccess.Read,FileShare.ReadWrite);
       });
       serverIdentity(account);
       WindowsIdentity.RunImpersonated(tok,delegate{
         if(File.ReadAllText(Path.Combine(root,"check.txt"))!=marker)throw new Exception("NAS user could not reopen fixture contents");
         string dst=Path.Combine(root,"check-role-"+Guid.NewGuid().ToString("N")+".txt");bool rejected=false;
         try{File.WriteAllText(dst,marker);}catch(UnauthorizedAccessException){rejected=true;}
         if(writable==rejected)throw new Exception((writable?"write unexpectedly denied":"read-only write unexpectedly allowed")+" with confirmed SMB identity");
         if(writable){if(File.ReadAllText(dst)!=marker)throw new Exception("NAS write/reopen mismatch");File.Delete(dst);}
       });
     }finally{if(held!=null)held.Dispose();
       if(connected)WindowsIdentity.RunImpersonated(tok,delegate{int rc=WNetCancelConnection2(root,0,false);if(rc!=0&&rc!=2250)throw new Exception("Cannot close own test session: "+rc);});}
   }
 }
 public static IntPtr FindOwned(int pid,string cls){var h=FindWindow(cls,null);uint actual;GetWindowThreadProcessId(h,out actual);return actual==pid?h:IntPtr.Zero;}
 public static IntPtr Item(IntPtr w,int id){var b=GetDlgItem(w,id);if(b==IntPtr.Zero)throw new Exception("Missing control "+id);return b;}
 public static void Click(IntPtr w,int id){if(!Post(Item(w,id),0xf5,IntPtr.Zero,IntPtr.Zero))throw new Exception("Click failed "+id);}
 public static void Set(IntPtr w,int id,string input){if(PutText(Item(w,id),12,IntPtr.Zero,input)==IntPtr.Zero)throw new Exception("Input failed "+id);}
 public static string Read(IntPtr w,int id){var b=new StringBuilder(16000);ReadText(Item(w,id),13,(IntPtr)b.Capacity,b);return b.ToString();}
 public static void Index(IntPtr w,int id,int i){if(Send(Item(w,id),0x14e,(IntPtr)i,IntPtr.Zero).ToInt64()!=i)throw new Exception("Option missing");}
}
