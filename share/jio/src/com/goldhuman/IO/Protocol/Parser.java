
package com.goldhuman.IO.Protocol;

import java.util.*;
import java.io.*;
import java.net.*;
import javax.xml.parsers.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;

public class Parser
{

	private static SAXParser sap;
	private static URL config = Parser.class.getResource("/config.xml");
	//private static URL config = ClassLoader.getSystemResource("config.xml");
	public static final String default_package =  "protocol";

	private static class Variable
	{
		public String name;
		public String type;

		public Variable(String name, String type)
		{
			this.name = name;
			this.type = type;
		}
	}
	public void setConfig(URL config)
	{
		this.config=config;
	}
	static
	{
		try
		{
			sap = SAXParserFactory.newInstance().newSAXParser();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	private static PrintStream find_file(String name)
	{
		String cname = default_package + "/" + name;
		File file = new File(cname + ".java");

		if (file.exists())
		{
			if (!new File( cname + ".class").exists())
				System.err.println("Compile " + cname + ".java first...");
			return null;
		}

		System.out.println("Generate " + file.getName());
		try
		{
			return  new PrintStream(new FileOutputStream(file));
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}

	private static void generate_import(PrintStream ps)
	{
		ps.println("package " + default_package + ";\n");
		ps.println("import com.goldhuman.Common.*;");
		ps.println("import com.goldhuman.Common.Marshal.*;");
		ps.println("import com.goldhuman.Common.Security.*;");
		ps.println("import com.goldhuman.IO.Protocol.*;");
		ps.println("");
	}

	private static void generate_datamember(PrintStream ps, Vector variables)
	{
		int size = variables.size();
		for (int i = 0; i < size; i ++)
		{
			Variable v = (Variable)variables.get(i);
			if(v.type.endsWith("Vector"))
				ps.println("\tpublic Rpc.Data.DataVector\t" + v.name + ";");
			else
				ps.println("\tpublic " + v.type + "\t" + v.name + ";");
		}
		ps.println("");
	}

	private static void generate_marshalable(PrintStream ps, Vector variables)
	{
		ps.println("\tpublic OctetsStream marshal(OctetsStream os)");
		ps.println("\t{");

		int size = variables.size();
		for (int i = 0; i < size; i ++)
		{
			Variable v = (Variable)variables.get(i);
			ps.println("\t\tos.marshal(" + v.name + ");");
		}

		ps.println("\t\treturn os;\n\t}");
		ps.println("");

		ps.println("\tpublic OctetsStream unmarshal(OctetsStream os) throws MarshalException");
		ps.println("\t{");

		for (int i = 0; i < size; i ++)
		{
			Variable v = (Variable)variables.get(i);
			String type = v.type;
			if (type.compareTo("int") == 0 || type.compareTo("short") == 0 || type.compareTo("long") == 0
				|| type.compareTo("byte") == 0 || type.compareTo("float") == 0 || type.compareTo("double") == 0 )
			{
				ps.println("\t\t" + v.name + " = os.unmarshal_" + type + "();");
			}
			else 
			{
				ps.println("\t\tos.unmarshal(" + v.name + ");");
			}
		}

		ps.println("\t\treturn os;\n\t}");
		ps.println("");
	}

	private static void generate_defaultconstructor(PrintStream ps, String name, Vector variables)
	{
		ps.println("\tpublic " + name + "()");
		ps.println("\t{");
		int size = variables.size();
		for (int i = 0; i < size; i ++)
		{
			Variable v = (Variable)variables.get(i);
			String type = v.type;
			String key = v.name;
			if (type.compareTo("int") != 0 && type.compareTo("short") != 0 && type.compareTo("long") != 0
				&& type.compareTo("byte") != 0 && type.compareTo("float") != 0 && type.compareTo("double") != 0 )
			{
				if(v.type.endsWith("Vector"))
					ps.println("\t\t" + key + " = new Rpc.Data.DataVector(" + "new " + type.replaceAll("Vector", "") + "());");
				else
					ps.println ("\t\t" + key + " = new " + type + "();");
			}
		}
		ps.println("\t}");
		ps.println("");
	}

	private static void generate_cloneable(PrintStream ps, String name, Vector variables)
	{
		ps.println("\tpublic Object clone()");
		ps.println("\t{");

		ps.println("\t\ttry");
		ps.println("\t\t{");
		ps.println("\t\t\t" + name + " o = (" + name + ")super.clone();");

		int size = variables.size();
		for (int i = 0; i < size; i ++)
		{
			Variable v = (Variable)variables.get(i);
			String type = v.type;
			String key = v.name;
			if (type.compareTo("int") != 0 && type.compareTo("short") != 0 && type.compareTo("long") != 0
				&& type.compareTo("byte") != 0 && type.compareTo("float") != 0 && type.compareTo("double") != 0 )
			{
				if(v.type.endsWith("Vector"))
					ps.println("\t\t\to." + key + " = (Rpc.Data.DataVector)" + key + ".clone();");
				else
					ps.println("\t\t\to." + key + " = (" + type + ")" + key + ".clone();");
			}
		}

		ps.println("\t\t\treturn o;");
		ps.println("\t\t}");
		ps.println("\t\tcatch (Exception e) { }");
		ps.println("\t\treturn null;\n\t}");
		ps.println("");
	}

	private static void generate_dataaccess(PrintStream ps, Vector variables)
	{
		int size = variables.size();
		for (int i = 0; i < size; i ++)
		{
			Variable v = (Variable)variables.get(i);
			String type = v.type;
			String key = v.name;
			ps.println("");
			ps.println("\tpublic void Set" + key + "(" + type + " " + key + ") { this." + key + " = " + key + "; }");
			ps.println("");
		}
	}

	private static void protocol_generate(String name, Vector variables)
	{
		PrintStream ps = find_file(name);
		if (ps == null) return;

		generate_import(ps);
		ps.println("public final class " + name + " extends Protocol");
		ps.println("{");
		generate_datamember(ps, variables);
		generate_defaultconstructor(ps, name, variables);
		generate_marshalable(ps, variables);
		generate_cloneable(ps, name, variables);
		//generate_dataaccess(ps, variables);

		ps.println("\tpublic void Process(Manager manager, Session session) throws ProtocolException");
		ps.println("\t{");
		ps.println("\t}");
		ps.println("");

		ps.println("}");
		ps.close();
	}

	private static void rpcdata_generate(String name, Vector variables)
	{
		PrintStream ps = find_file(name);
		if (ps == null) return;

		generate_import(ps);
		ps.println("public final class " + name + " extends Rpc.Data");
		ps.println("{");
		generate_datamember(ps, variables);
		generate_defaultconstructor(ps, name, variables);
		generate_marshalable(ps, variables);
		generate_cloneable(ps, name, variables);
		//generate_dataaccess(ps, variables);
		ps.println("}");
		ps.close();
	}

	private static void rpc_generate(String name, String argument, String result)
	{
		PrintStream ps = find_file(name);
		if (ps == null) return;

		generate_import(ps);
		ps.println("public final class " + name + " extends Rpc");
		ps.println("{");

		ps.println("\tpublic void Server(Data argument, Data result) throws ProtocolException");
		ps.println("\t{");
		ps.println("\t\t"+argument+" arg = ("+argument+")argument;"); 
		ps.println("\t\t"+result+" res = ("+result+")result;"); 
		ps.println("\t}");
		ps.println("");

		ps.println("\tpublic void Client(Data argument, Data result) throws ProtocolException");
		ps.println("\t{");
		ps.println("\t\t"+argument+" arg = ("+argument+")argument;"); 
		ps.println("\t\t"+result+" res = ("+result+")result;"); 
		ps.println("\t}");
		ps.println("");

		ps.println("\tpublic void OnTimeout()");
		ps.println("\t{");
		ps.println("\t}");
		ps.println("");

		ps.println("}");
		ps.close();
	}

	protected static void ParseProtocol(final Map map) throws Exception
	{
		sap.parse(config.openStream(),
			new DefaultHandler()
			{
				private boolean parsing = false;
				private String name;
				private String type;
				private String class_name;
				private String maxsize;
				private String priority;
				private Vector variables;
				public void startElement(String uri, String localName, String qName, Attributes attributes)
				{
					try
					{
						if (qName.compareTo("protocol") == 0)
						{
							parsing = true;
							variables  = new Vector();

							name       = attributes.getValue("name");
							type       = attributes.getValue("type");
							class_name = attributes.getValue("class");
							maxsize    = attributes.getValue("maxsize");
							priority   = attributes.getValue("priority");
							if( priority == null )
								priority = attributes.getValue("prior");
						}
						else if (parsing && qName.compareTo("variable") == 0)
						{
							String vtype = attributes.getValue("type");
							vtype = vtype.replaceAll("unsigned","").trim();
							vtype = vtype.replaceAll("char","byte").trim();
							variables.add(new Variable(attributes.getValue("name"),vtype));
						}
					}
					catch (Exception e)
					{
						e.printStackTrace();
					}
				}

				public void endElement(String uri, String localName, String qName)
				{
					if (parsing && qName.compareTo("protocol") == 0)
					{
						parsing = false;
						try
						{
							Protocol instance = (Protocol)Class.
								forName(default_package + "." + (class_name == null ? name : class_name)).newInstance();
							instance.type = Integer.parseInt(type);
							instance.size_policy = (maxsize == null ? 0 : Integer.parseInt(maxsize));
							instance.prior_policy = (priority == null ? 0 : Integer.parseInt(priority));
							if (map.put(name.toUpperCase(), instance) != null)
								System.err.println("Duplicate protocol name " + name);
							if (map.put(type, instance) != null)
								System.err.println("Duplicate protocol type " + type);
						}
						catch (Exception e)
						{
							protocol_generate(name, variables);
						}

						name       = null;
						type       = null;
						class_name = null;
						maxsize    = null;
						priority   = null;
						variables  = null;
					}
				}

			}
		);
	}

	protected static void ParseState(final Map map) throws Exception
	{
		final Map typemap = new HashMap();

		sap.parse(config.openStream(),
			new DefaultHandler()
			{
				public void startElement(String uri, String localName, String qName, Attributes attributes)
				{
					if (qName.compareTo("protocol") != 0 && qName.compareTo("rpc") != 0)
						return;
					try
					{
						typemap.put(attributes.getValue("name").trim(), attributes.getValue("type").trim());
					}
					catch (Exception e)
					{
						e.printStackTrace();
					}
				}
			}
		);

		sap.parse(config.openStream(),
			new DefaultHandler()
			{
				private String state_name = null;
				private State  state = null;
				public void startElement(String uri, String localName, String qName, Attributes attributes)
				{
					if (qName.compareTo("state") == 0)
					{
						try
						{
							state_name = attributes.getValue("name").trim().toUpperCase();
							try
							{
								state = new State(
									1000 * Long.parseLong(
										attributes.getValue("timeout").trim()));
							}
							catch(Exception e)
							{
								state = new State(0);
							}
						}
						catch(Exception e)
						{
						}
					}
					if (state_name != null && qName.compareTo("proto") == 0){
						state.AddProtocolType((String)typemap.get(attributes.getValue("name")));
					}
				}

				public void endElement(String uri, String localName, String qName)
				{
					if (qName.compareTo("state") == 0)
					{
						if (state_name != null)
							map.put(state_name, state);
						state_name = null;
					}
				}

			}
		);
	}

	private static final Map ParseRpcData() throws Exception
	{
		final Map typemap = new HashMap();
		sap.parse(config.openStream(),
			new DefaultHandler()
			{
				private boolean parsing = false;
				private String rpcdataName = null;
				private Vector variables;

				public void startElement(String uri, String localName, String qName, Attributes attributes) 
				{
					try
					{
						if (qName.compareTo("rpcdata") == 0)
						{
							parsing = true;
							variables = new Vector();

							rpcdataName = attributes.getValue("name");
						}
						else if (parsing && qName.compareTo("variable") == 0)
						{
							String vtype = attributes.getValue("type");
							vtype = vtype.replaceAll("unsigned","").trim();
							vtype = vtype.replaceAll("char","byte").trim();
							variables.add(new Variable(attributes.getValue("name"),vtype));
						}
					}
					catch(Exception e)
					{
						e.printStackTrace();
					}
				}

				public void endElement(String uri, String localName, String qName)
				{
					if (parsing && qName.compareTo("rpcdata") == 0)
					{
						parsing = false;
						try
						{
							typemap.put(rpcdataName, Class.forName(default_package + "." + rpcdataName).newInstance());
							
						}
						catch(Exception e)
						{
							rpcdata_generate(rpcdataName, variables);
						}

						rpcdataName = null;
						variables = null;
					}
				}
			}
		);
		return typemap; 
	}

	protected static void ParseRpc(final Map map) throws Exception
	{
		final Map typemap = ParseRpcData();

		sap.parse(config.openStream(),
			new DefaultHandler()
			{
				public void startElement(String uri, String localName, String qName, Attributes attributes) 
				{
					if (qName.compareTo("rpc") == 0)
					{
						try
						{
							String name     = attributes.getValue("name");
							String type     = attributes.getValue("type");
							String argument = attributes.getValue("argument");
							String result   = attributes.getValue("result");
							String maxsize  = attributes.getValue("maxsize");
							String timeout  = attributes.getValue("timeout");
							String priority = attributes.getValue("priority");
							if( priority == null )
								priority = attributes.getValue("prior");

							Rpc rpc = null;
							try
							{
								rpc = (Rpc)Class.forName(default_package + "." + name).newInstance();
							}
							catch(Exception e1)
							{
								rpc_generate(name, argument, result);	
								return;
							}
							rpc.argument = (Rpc.Data)typemap.get(argument);
							rpc.result = (Rpc.Data)typemap.get(result);
							rpc.type = Integer.parseInt(type);
							rpc.prior_policy = (priority == null ? 0 : Integer.parseInt(priority));

							rpc.time_policy = (timeout == null ? 25000l : 1000l * Long.parseLong(timeout));
							rpc.size_policy = (maxsize == null ? 0 : Integer.parseInt(maxsize));

							if (map.put(name.toUpperCase(), rpc) != null)
								System.err.println("Duplicate protocol name " + name);
							if (map.put(type, rpc) != null)
								System.err.println("Duplicate protocol type " + type);
						}
						catch(Exception e)
						{
							e.printStackTrace();
						}
					}
				}
			}
		);
	}
}
