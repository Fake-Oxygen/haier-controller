--
-- PostgreSQL database dump
--

\restrict bZzjVcGK6Qk7steaczDb4HtUifG8swaD2fYsDTG7IhbLGy8ZLlPcAKeEgFbJ90Q

-- Dumped from database version 18.4
-- Dumped by pg_dump version 18.4

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: status; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.status (
    id bigint NOT NULL,
    "timestamp" timestamp with time zone,
    ambient_temp real,
    ch_temp real,
    dhw_temp real,
    ch_target_temp real,
    dhw_target_temp real,
    opr_mode smallint,
    valve_state smallint,
    tank_state smallint,
    heater_state smallint
);


ALTER TABLE public.status OWNER TO postgres;

--
-- Name: status_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.status ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.status_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: status status_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.status
    ADD CONSTRAINT status_pkey PRIMARY KEY (id);


--
-- Name: TABLE status; Type: ACL; Schema: public; Owner: postgres
--

GRANT INSERT ON TABLE public.status TO morow;


--
-- Name: SEQUENCE status_id_seq; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,USAGE ON SEQUENCE public.status_id_seq TO morow;


--
-- PostgreSQL database dump complete
--

\unrestrict bZzjVcGK6Qk7steaczDb4HtUifG8swaD2fYsDTG7IhbLGy8ZLlPcAKeEgFbJ90Q

