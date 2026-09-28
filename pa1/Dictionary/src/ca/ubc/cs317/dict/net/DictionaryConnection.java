package ca.ubc.cs317.dict.net;

import ca.ubc.cs317.dict.model.Database;
import ca.ubc.cs317.dict.model.Definition;
import ca.ubc.cs317.dict.model.MatchingStrategy;

// import java.io.BufferedReader;
// import java.io.PrintWriter;
// import java.net.Socket;
import java.util.*;

import java.io.*;
import java.net.*;

/**
 * Created by Jonatan on 2017-09-09.
 */
public class DictionaryConnection {

    private static final int DEFAULT_PORT = 2628;
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;

    /** Establishes a new connection with a DICT server using an explicit host and port number, and handles initial
     * welcome messages.
     *
     * @param host Name of the host where the DICT server is running
     * @param port Port number used by the DICT server
     * @throws DictConnectionException If the host does not exist, the connection can't be established, or the messages
     * don't match their expected value.
     */
    public DictionaryConnection(String host, int port) throws DictConnectionException {
        // TODO Replace this with code that creates the requested connection
        // throw new DictConnectionException("Not implemented");
        
	try {
	    this.socket = new Socket(host, port);
	    this.out = new PrintWriter(socket.getOutputStream(), true);
	    this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

	    String response = in.readLine();
	    if (response.isEmpty()) throw new DictConnectionException("Did not receive initial message from server.");

	    // Handle negative replies
	    if (response.startsWith("4")) throw new DictConnectionException("Received Transient Negative Completion reply.");
	    if (response.startsWith("5")) throw new DictConnectionException("Received Permanent Negative Completion reply.");
	} catch (DictConnectionException e) {
            throw e;
	} catch (UnknownHostException e) {
            throw new DictConnectionException("Don't know about host " + host);
	} catch (IOException e) {
	    throw new DictConnectionException("IO error when creating socket. ");
	} catch (Exception e) {
	    throw new DictConnectionException("Something else went wrong when creating a socket.");
	}

        System.out.printf("Connected to %s at port %d. \n", host, port);
    }

    /** Establishes a new connection with a DICT server using an explicit host, with the default DICT port number, and
     * handles initial welcome messages.
     *
     * @param host Name of the host where the DICT server is running
     * @throws DictConnectionException If the host does not exist, the connection can't be established, or the messages
     * don't match their expected value.
     */
    public DictionaryConnection(String host) throws DictConnectionException {
        this(host, DEFAULT_PORT);
    }

    /** Sends the final QUIT message and closes the connection with the server. This function ignores any exception that
     * may happen while sending the message, receiving its reply, or closing the connection.
     *
     */
    public synchronized void close() {

        // TODO Add your code here
	try {
	    this.out.println("QUIT");
	    this.socket.close();
	} catch (Exception e) {}

	System.out.println("Closed connection.");
    }

    /** Requests and retrieves all definitions for a specific word.
     *
     * @param word The word whose definition is to be retrieved.
     * @param database The database to be used to retrieve the definition. A special database may be specified,
     *                 indicating either that all regular databases should be used (database name '*'), or that only
     *                 definitions in the first database that has a definition for the word should be used
     *                 (database '!').
     * @return A collection of Definition objects containing all definitions returned by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Collection<Definition> getDefinitions(String word, Database database) throws DictConnectionException {
        Collection<Definition> set = new ArrayList<>();

        // TODO Add your code here
	try {
	    this.out.printf("DEFINE %s %s\n", database.getName(), word);

	    String response = this.in.readLine();
	    if (response.startsWith("550")) {
		System.out.println("Invalid database.");
		return set;
	    }
	    if (response.startsWith("552")) {
		System.out.println("No match found.");
		return set;
	    }
	    if (!response.startsWith("150")) throw new DictConnectionException("Invalid response from server.");

	    response = this.in.readLine();
	    while (!response.startsWith("250")) {
		String[] parsed = response.split("\"");
		Definition newDef = new Definition(parsed[1].trim(), parsed[2].trim());

		// Read the definition
		response = this.in.readLine();
		while (!response.startsWith(".")) {
		    newDef.appendDefinition(response);
		    response = this.in.readLine();
		}

		set.add(newDef);
		response = this.in.readLine();
	    }

	    if (!response.startsWith("250")) throw new DictConnectionException("Did not get 250 ok from server.");

	} catch (DictConnectionException e) {
	    throw e;
	} catch (Exception e) {
	    throw new DictConnectionException("Something else went wrong when retrieving definition.");
	}

        return set;
    }

    /** Requests and retrieves a list of matches for a specific word pattern.
     *
     * @param word     The word whose definition is to be retrieved.
     * @param strategy The strategy to be used to retrieve the list of matches (e.g., prefix, exact).
     * @param database The database to be used to retrieve the definition. A special database may be specified,
     *                 indicating either that all regular databases should be used (database name '*'), or that only
     *                 matches in the first database that has a match for the word should be used (database '!').
     * @return A set of word matches returned by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Set<String> getMatchList(String word, MatchingStrategy strategy, Database database) throws DictConnectionException {
        Set<String> set = new LinkedHashSet<>();

        // TODO Add your code here
	try {
	    this.out.printf("MATCH %s %s %s\n", database.getName(), strategy.getName(), word);

	    String response = this.in.readLine();
	    if (response.startsWith("550")) {
		System.out.println("Invalid database.");
		return set;
	    }
	    if (response.startsWith("551")) {
		System.out.println("Invalid strategy.");
		return set;
	    }
	    if (response.startsWith("552")) {
		System.out.println("No match found.");
		return set;
	    }

	    if (!response.startsWith("152")) throw new DictConnectionException("Invalid response from server.");

	    response = this.in.readLine();
	    while (!response.startsWith(".")) {
		int splitPos = response.indexOf(" ");
		String match = response.substring(splitPos + 1).replaceAll("\"", "");
		set.add(match);
		response = this.in.readLine();
	    }

            if (!this.in.readLine().startsWith("250")) throw new DictConnectionException("Did not get 250 ok from server.");

        } catch (DictConnectionException e) {
	    throw e;
	} catch (Exception e) {
	    throw new DictConnectionException("Something else went wrong when matching a word.");
	}

	System.out.printf("Matched %s with strategy %s in database %s.\n", word, strategy.getName(), database.getName());
        return set;
    }

    /** Requests and retrieves a map of database name to an equivalent database object for all valid databases used in the server.
     *
     * @return A map of Database objects supported by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Map<String, Database> getDatabaseList() throws DictConnectionException {
        Map<String, Database> databaseMap = new HashMap<>();

        // TODO Add your code here
	try {
	    this.out.println("SHOW DATABASES");
	    
	    String response = this.in.readLine();
	    // Check if reponse code is 110 or 554
	    if (response.startsWith("554")) return databaseMap;
	    if (!response.startsWith("110")) throw new DictConnectionException("Invalid response from server.");

	    response = this.in.readLine();
            while (!response.equals(".")) {
		// Parse the responses
		int splitPos = response.indexOf(" ");
		String dbName = response.substring(0, splitPos);
		String dbInfo = response.substring(splitPos + 1);
		dbInfo = dbInfo.replaceAll("\"", "");
		Database newDb = new Database(dbName, dbInfo);
		databaseMap.put(dbName, newDb);

		response = this.in.readLine();
	    }

	    if (!this.in.readLine().startsWith("250")) throw new DictConnectionException("Did not get 250 ok from server.");

	} catch (DictConnectionException e) {
            throw e;
	} catch (IOException e) {
	    throw new DictConnectionException("IO error when retrieving databases.");
	} catch (Exception e) {
	    throw new DictConnectionException("Something else went wrong when retrieving databases.");
	}

	System.out.println("Retrieved databases.");
	return databaseMap;

    }

    /** Requests and retrieves a list of all valid matching strategies supported by the server.
     *
     * @return A set of MatchingStrategy objects supported by the server.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized Set<MatchingStrategy> getStrategyList() throws DictConnectionException {
        Set<MatchingStrategy> set = new LinkedHashSet<>();

        // TODO Add your code here
	try {
	    this.out.println("SHOW STRATEGIES");
	    String response = this.in.readLine();
	    if (response.startsWith("555")) return set;
	    if (!response.startsWith("111")) throw new DictConnectionException("Invalid response when retrieving strategies.");

	    response = this.in.readLine();
	    while (!response.startsWith(".")) {
		int splitPos = response.indexOf(" ");
		String stratName = response.substring(0, splitPos);
		String stratInfo = response.substring(splitPos + 1);
		stratInfo = stratInfo.replaceAll("\"", "");

		MatchingStrategy newStrat = new MatchingStrategy(stratName, stratInfo);
		set.add(newStrat);

		response = this.in.readLine();
	    }

	    if (!this.in.readLine().startsWith("250")) throw new DictConnectionException("Did not get 250 ok from server.");

	} catch (DictConnectionException e) {
	    throw e;
	} catch (Exception e) {
	    throw new DictConnectionException("Something else went wrong when retrieving strategies.");
	}

	System.out.println("Retrieved strategies.");
        return set;
    }

    /** Requests and retrieves detailed information about the currently selected database.
     *
     * @return A string containing the information returned by the server in response to a "SHOW INFO <db>" command.
     * @throws DictConnectionException If the connection was interrupted or the messages don't match their expected value.
     */
    public synchronized String getDatabaseInfo(Database d) throws DictConnectionException {
	StringBuilder sb = new StringBuilder();

        // TODO Add your code here
        try {
	    this.out.println("SHOW INFO " + d.getName());
	    String response = this.in.readLine();

	    if (response.startsWith("550")) throw new DictConnectionException("Invalid database for SHOW INFO.");

	    response = this.in.readLine();

	    while (!response.startsWith(".")) {
		sb.append(response + "\n");
		response = this.in.readLine();
	    }
	    
            if (!this.in.readLine().startsWith("250")) throw new DictConnectionException("Did not get 250 ok from server.");
	} catch (DictConnectionException e) {
	    throw e;
	} catch (Exception e) {
	    throw new DictConnectionException("Something else went wrong when getting database info.");
	}

	System.out.printf("Retrieved info for database %s.\n", d.getName());
	return sb.toString();
 
    }
}
